project(EventUtils)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package( OpenCV 5.0.0 REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_library(${PROJECT_NAME} STATIC EventContainer.cpp OpsEvent.cpp EventHelper.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES EventContainer.hpp OpsEvent.hpp EventHelper.hpp DESTINATION include)