project(EventUtils)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package( OpenCV 5.0.0 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

add_library(${PROJECT_NAME} STATIC EventContainer.cpp OpsEvent.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES EventContainer.hpp OpsEvent.hpp DESTINATION include)