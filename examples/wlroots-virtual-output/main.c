// Minimal wlroots compositor demonstrating a headless (virtual) output.
//
// This creates a wlroots compositor, adds a headless output, and renders a
// simple color on each frame.

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <wayland-server.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_output.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/backend.h>
#include <wlr/backend/headless.h>
#include <wlr/render/allocator.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static struct wl_display *display;
static struct wlr_backend *backend;
static struct wlr_renderer *renderer;
static struct wlr_allocator *allocator;
static struct wlr_output_layout *output_layout;

struct headless_output {
    struct wlr_output *output;
    struct wl_listener frame;
    struct wl_listener destroy;
};

static void handle_frame(struct wl_listener *listener, void *data) {
    struct headless_output *ho = wl_container_of(listener, ho, frame);
    struct wlr_output *output = ho->output;

    int buffer_age = 0;
    if (!wlr_output_attach_render(output, &buffer_age)) {
        return;
    }

    struct wlr_renderer *renderer = output->renderer;
    wlr_renderer_begin(renderer, output->width, output->height);

    // Simple color animation based on frame count
    static uint32_t frame_count = 0;
    float t = (frame_count++ % 360) / 360.0f;
    float r = 0.2f + 0.4f * (1.0f + sinf(t * 2.0f * M_PI));
    float g = 0.2f + 0.4f * (1.0f + cosf(t * 2.0f * M_PI));
    float b = 0.2f;
    float clear_color[4] = {r, g, b, 1.0f};
    wlr_renderer_clear(renderer, clear_color);

    wlr_renderer_end(renderer);

    wlr_output_set_damage(output, NULL);
    wlr_output_commit(output);
}

static void handle_destroy(struct wl_listener *listener, void *data) {
    struct headless_output *ho = wl_container_of(listener, ho, destroy);
    wl_list_remove(&ho->frame.link);
    wl_list_remove(&ho->destroy.link);
    free(ho);
}

static struct headless_output *create_headless_output(struct wlr_output_layout *layout,
        struct wlr_backend *backend, uint32_t width, uint32_t height) {
    struct headless_output *ho = calloc(1, sizeof(*ho));
    if (!ho) {
        return NULL;
    }

    ho->output = wlr_headless_add_output(backend, width, height);
    if (!ho->output) {
        free(ho);
        return NULL;
    }

    if (!wlr_output_init_render(ho->output, allocator, renderer)) {
        free(ho);
        return NULL;
    }

    wlr_output_set_custom_mode(ho->output, width, height, 60000);
    wlr_output_enable(ho->output, true);
    wlr_output_create_global(ho->output);

    wl_signal_add(&ho->output->events.frame, &ho->frame);
    wl_signal_add(&ho->output->events.destroy, &ho->destroy);
    ho->frame.notify = handle_frame;
    ho->destroy.notify = handle_destroy;

    wlr_output_layout_add_auto(layout, ho->output);
    wlr_output_commit(ho->output);
    return ho;
}

static volatile sig_atomic_t should_terminate = 0;

static void signal_handler(int signum) {
    (void)signum;
    should_terminate = 1;
}

static int timer_handler(void *data) {
    if (!should_terminate) {
        return 1;
    }
    wl_display_terminate(data);
    return 0;
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    display = wl_display_create();
    if (!display) {
        fprintf(stderr, "Failed to create Wayland display\n");
        return 1;
    }

    const char *socket = wl_display_add_socket_auto(display);
    if (!socket) {
        fprintf(stderr, "Failed to create Wayland socket\n");
        return 1;
    }

    backend = wlr_headless_backend_create(display);
    if (!backend) {
        fprintf(stderr, "Failed to create headless backend\n");
        return 1;
    }

    renderer = wlr_renderer_autocreate(backend);
    if (!renderer) {
        fprintf(stderr, "Failed to create renderer\n");
        return 1;
    }

    wlr_renderer_init_wl_display(renderer, display);

    allocator = wlr_allocator_autocreate(backend, renderer);
    if (!allocator) {
        fprintf(stderr, "Failed to create allocator\n");
        return 1;
    }

    output_layout = wlr_output_layout_create();
    if (!output_layout) {
        fprintf(stderr, "Failed to create output layout\n");
        return 1;
    }

    struct wlr_compositor *compositor = wlr_compositor_create(display, renderer);
    if (!compositor) {
        fprintf(stderr, "Failed to create compositor\n");
        return 1;
    }

    // Create a single headless output
    struct headless_output *ho = create_headless_output(output_layout, backend, 1280, 720);
    if (!ho) {
        fprintf(stderr, "Failed to create headless output\n");
        return 1;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    struct wl_event_loop *loop = wl_display_get_event_loop(display);
    struct wl_event_source *timer = wl_event_loop_add_timer(loop, timer_handler, display);
    wl_event_source_timer_update(timer, 100);

    if (!wlr_backend_start(backend)) {
        fprintf(stderr, "Failed to start backend\n");
        return 1;
    }

    printf("Virtual output created. Run a Wayland client and point it at this display.\n");
    printf("Set WAYLAND_DISPLAY=%s and run a client.\n", socket);

    wl_display_run(display);

    wlr_backend_destroy(backend);
    wl_display_destroy(display);
    return 0;
}
