cmake_minimum_required(VERSION 3.31)
project(fbxViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

find_package(Vulkan REQUIRED)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# ImGui (core only; backends compiled into our target)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
)
FetchContent_MakeAvailable(imgui)

# ofbx (tiny FBX parser)
FetchContent_Declare(
    ofbx
    GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
    GIT_TAG master
)
FetchContent_MakeAvailable(ofbx)

file(GLOB_RECURSE FBXVIEWER_SOURCES
    CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
)

add_executable(fbxViewer
    main.cpp
    ${FBXVIEWER_SOURCES}
    # ImGui backends compiled into our target for simplicity
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

target_include_directories(fbxViewer PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${ofbx_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(fbxViewer PRIVATE
    Vulkan::Vulkan
    glfw
    glm
)

if(MSVC)
    target_compile_definitions(fbxViewer PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# stb_image header-only include (vendor minimal)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)
target_link_libraries(fbxViewer PRIVATE stb_image)
