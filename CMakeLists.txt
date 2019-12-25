cmake_minimum_required(VERSION 3.15)
project(stream)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModule/)

find_package(OpenCV REQUIRED)
find_package(FFmpeg)
if(FFMPEG_FOUND)
    message(STATUS "FOUND!!")
    include_directories(${FFMPEG_INCLUDE_DIRS})
endif()

include_directories(
        ${OpenCV_INCLUDE_DIRS}
)

add_executable(
        stream
        main.cpp
)

target_link_libraries(
        stream
        ${FFMPEG_LIBRARIES}
        ${OpenCV_LIBS}
        )