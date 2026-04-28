cmake_minimum_required(VERSION 3.20)
project(esp32_scope LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
find_package(glfw3 3.3 REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

# ImGui
add_library(imgui
    extern/imgui/imgui.cpp
    extern/imgui/imgui_draw.cpp
    extern/imgui/imgui_tables.cpp
    extern/imgui/imgui_widgets.cpp
    extern/imgui/backends/imgui_impl_glfw.cpp
    extern/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    extern/imgui
    extern/imgui/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# ImPlot
add_library(implot
    extern/implot/implot.cpp
    extern/implot/implot_items.cpp
)

target_include_directories(implot PUBLIC
    extern/implot
    extern/imgui
)

target_link_libraries(implot PUBLIC imgui)

# glad
add_library(glad
    extern/glad/src/glad.c
)

target_include_directories(glad PUBLIC
    extern/glad/include
)

# App
add_executable(esp32_scope
    src/main.cpp
    src/udp_receiver.cpp
)

target_include_directories(esp32_scope PRIVATE
    include
)

target_link_libraries(esp32_scope PRIVATE
    glad
    imgui
    implot
    glfw
    OpenGL::GL
)

if (WIN32)
    target_link_libraries(esp32_scope PRIVATE ws2_32)
endif()