cmake_minimum_required(VERSION 3.17)
project(Researches_upon_Task_Scheduling_of_Heterogeneous_Parallel_System
        HOMEPAGE_URL "https://github.com/Rossil2012/Researches-upon-Task-Scheduling-of-Heterogeneous-Parallel-System")

set(CMAKE_CXX_STANDARD 14)

set(ALGORITHM_SRC
        src/algorithm/helper/helper.cc
        src/algorithm/ga/ga.cc
        src/algorithm/ga/ga_nl.cc
        src/algorithm/ga/ga_pa.cc
        src/algorithm/ls/ls.cc
        src/algorithm/ls/ls_nl.cc
        src/algorithm/ls/ls_pa.cc
        src/algorithm/ls/ls_asap.cc
        src/algorithm/ls/ls_alap.cc
        src/algorithm/naive/hashing.cc
        src/algorithm/naive/cpop.cc)

set(STRUCTS_SRC
        src/structs/graph/graph.cc
        src/structs/task/task.cc
        src/structs/device/device.cc)

add_library(algorithm ${ALGORITHM_SRC})
add_library(structs ${STRUCTS_SRC})
add_library(global src/global.cc)

add_executable(main
        src/main/main.cc)

target_link_libraries(main PRIVATE
        global
        algorithm
        structs)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/algorithm
        ${PROJECT_SOURCE_DIR}/include/structs)