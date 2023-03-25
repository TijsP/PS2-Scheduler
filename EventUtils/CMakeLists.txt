project(EventUtils)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package( OpenCV 5.0.0 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

add_library(EventUtils STATIC EventContainer.cpp OpsEvent.cpp)
target_link_libraries( EventUtils PUBLIC ${OpenCV_LIBS} )