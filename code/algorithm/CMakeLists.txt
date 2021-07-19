project(algorithm)

set(SOURCES
        src/ga.cc)

add_library(${PROJECT_NAME} ${SOURCES})
add_library(schedule::algorithm ALIAS ${PROJECT_NAME})

include_directories(../structs/include)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)