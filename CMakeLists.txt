cmake_minimum_required(VERSION 3.14)

project(floodfill_fun)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SOURCE_DIR "src")
set(SOURCES ${SOURCE_DIR}/main.cpp)
set(DO_OPENCV_OUTPUT 0)

add_executable(floodfill_fun ${SOURCES})

if (DO_OPENCV_OUTPUT)
    add_definitions(-DOPENCV_OUTPUT)
    find_package(OpenCV CONFIG REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})
    target_link_libraries(main ${OpenCV_LIBS})
endif()

include_directories(
    ${SOURCE_DIR}
)
