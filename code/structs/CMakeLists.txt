project(structs)

set(SOURCES
        src/graph/graph.cc
        src/task/task.cc
        src/device/device.cc
        )

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(schedule::structs ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)