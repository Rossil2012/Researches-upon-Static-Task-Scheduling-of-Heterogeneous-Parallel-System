project(structs)

set(SOURCES
        src/graph.cc
        src/task.cc
        src/device.cc)

add_library(${PROJECT_NAME} ${SOURCES})
add_library(schedule::structs ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)