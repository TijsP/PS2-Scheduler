cmake_minimum_required(VERSION 3.0.0)
project(PS2-Scheduler VERSION 0.2.0)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

aux_source_directory(ImGUI imgui)
aux_source_directory(TinyFD tinyfd)

find_package( glfw3 REQUIRED)
find_package( OpenCV 5.0.0 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
find_package( OpenGL REQUIRED)
add_subdirectory(EventUtils)

add_compile_definitions(SCHEDULER_VERSION="${PROJECT_VERSION}")

add_executable(${PROJECT_NAME} main.cpp ${imgui} ${tinyfd} ${PROJECT_SOURCE_DIR}/icon.rc)

target_link_libraries(${PROJECT_NAME} EventUtils ${OpenCV_LIBS} glfw OpenGL::GL)
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/EventUtils" )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> DESTINATION bin)
install(FILES "TXLC_Planning.png" DESTINATION bin/..)

include(InstallRequiredSystemLibraries)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "PS2 Squad Scheduler")
include(CPack)
