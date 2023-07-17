cmake_minimum_required (VERSION 3.0)

project(avm_app)

set(OpenCV_DIR "D:/Software/opencv/opencv/build") 

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

message(STATUS "opencv inc path " ${OpenCV_INCLUDE_DIRS})

include_directories(./srcs)

set (
    FILE_SRCS
    main.cpp
)

#file(GLOB_RECURSE COMMON_FILES ./common/*.cpp ./common/*.h*)

#file(GLOB_RECURSE MODULES_FILES ./modules/*.cpp ./moudles/*.h*)

#list(APPEND FILE_SRCS ${COMMON_FILES} ${MODULES_FILES})

message (STATUS "get file srcs lits: " ${FILE_SRCS})

add_executable (avm_cali avm_cali_demo.cpp ./srcs/common.cpp)
add_executable (avm_app avm_app_demo.cpp ./srcs/common.cpp)

include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARY_DIRS})

target_link_libraries(avm_cali ${OpenCV_LIBS})
target_link_libraries(avm_app ${OpenCV_LIBS})