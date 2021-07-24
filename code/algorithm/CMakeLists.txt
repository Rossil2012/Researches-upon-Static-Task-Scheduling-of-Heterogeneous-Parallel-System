project(algorithm)

set(SOURCES
        src/helper/helper.cc
        src/ga/ga.cc
        src/ga/ga_pa.cc
        src/ga/ga_nl.cc
        src/ls/ls.cc
        src/ls/ls_nl.cc)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(schedule::algorithm ALIAS ${PROJECT_NAME})

include_directories(../structs/include)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${PROJECT_SOURCE_DIR}/include)