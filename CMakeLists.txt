cmake_minimum_required(VERSION 3.20)
project(cpp-project CXX)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${CMAKE_SOURCE_DIR}/lib/glfw")

file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/include/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(cpp-project ${SRC_FILES} ${IMGUI_SOURCES})

set_target_properties(cpp-project PROPERTIES CXX_STANDARD 23)

target_include_directories(cpp-project PUBLIC 
    "${CMAKE_SOURCE_DIR}/include"
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
)

target_link_libraries(cpp-project PRIVATE glfw opengl32)

if(MINGW)
  target_link_libraries(cpp-project PRIVATE "-lstdc++exp")
endif()