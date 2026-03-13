# wlroots Headless Output Example

This is a minimal standalone compositor that creates a **headless (virtual) output** using `wlroots`.

It demonstrates the simplest way to:

1. Create a Wayland compositor using wlroots
2. Create a headless output (an in-memory virtual monitor)
3. Render simple content into the headless output every frame

## Build

Requirements:
- wlroots (with headless backend support)
- wayland-server development headers
- CMake 3.22+ (or build manually with gcc + pkg-config)
- Ninja (or any other generator; this example uses Ninja)

From the repo root:

```bash
mkdir -p build-wlroots
cd build-wlroots
cmake -G Ninja -S examples/wlroots-virtual-output -B .
ninja
```

## Run

```bash
./wlroots-virtual-output
```

It will create a virtual output named `Virtual-0` (visible to Wayland clients as a monitor).

This example is intended for experimentation and learning; it is not a full compositor.
