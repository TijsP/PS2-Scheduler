cmake_minimum_required(VERSION 3.0.0)
project(PS2-Scheduler VERSION 0.1.0)

include(CTest)
enable_testing()

set (CMAKE_CXX_FLAGS "-std:c++17")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(glfw3_DIR "C:\\Program Files\\GLFW\\lib\\cmake\\glfw3")

aux_source_directory(ImGUI imgui)
aux_source_directory(TinyFD tinyfd)

find_package( glfw3 REQUIRED)
find_package( OpenCV 5.0.0 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
find_package( OpenGL REQUIRED)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
add_executable(PS2-Scheduler main.cpp ${imgui} ${tinyfd})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} glfw OpenGL::GL )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
