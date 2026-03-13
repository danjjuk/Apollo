cmake_minimum_required(VERSION 3.22)
project(wlroots_virtual_output_example LANGUAGES C)

find_package(PkgConfig REQUIRED)

pkg_check_modules(WLROOTS REQUIRED wlroots)
pkg_check_modules(WAYLAND_SERVER REQUIRED wayland-server)

add_executable(wlroots-virtual-output main.c)

target_include_directories(wlroots-virtual-output PRIVATE
    ${WLROOTS_INCLUDE_DIRS}
    ${WAYLAND_SERVER_INCLUDE_DIRS}
)

target_compile_definitions(wlroots-virtual-output PRIVATE WLR_USE_UNSTABLE)

target_link_libraries(wlroots-virtual-output PRIVATE
    ${WLROOTS_LIBRARIES}
    ${WAYLAND_SERVER_LIBRARIES}
    m
)

target_compile_options(wlroots-virtual-output PRIVATE
    ${WLROOTS_CFLAGS_OTHER}
    ${WAYLAND_SERVER_CFLAGS_OTHER}
)

target_link_options(wlroots-virtual-output PRIVATE
    ${WLROOTS_LDFLAGS_OTHER}
    ${WAYLAND_SERVER_LDFLAGS_OTHER}
)
