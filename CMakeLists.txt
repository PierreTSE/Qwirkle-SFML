cmake_minimum_required(VERSION 3.0)
project(Qwirkle-SFML)

# version number
set(QWIRKLE_SFML_MAJOR 1)
set(QWIRKLE_SFML_MINOR 2)
set(QWIRKLE_SFML_PATCH 1)
set(QWIRKLE_SFML_VERSION ${QWIRKLE_SFML_MAJOR}.${QWIRKLE_SFML_MINOR}.${QWIRKLE_SFML_PATCH})
add_compile_definitions(QWIRKLE_SFML_MAJOR=${QWIRKLE_SFML_MAJOR}
        QWIRKLE_SFML_MINOR=${QWIRKLE_SFML_MINOR}
        QWIRKLE_SFML_PATCH=${QWIRKLE_SFML_PATCH})

set(EXECUTABLE_NAME "Qwirkle-${QWIRKLE_SFML_VERSION}")

set(CMAKE_CXX_STANDARD 17)
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -Wno-switch")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3")
endif ()

file(GLOB_RECURSE SOURCE_FILES src/*)
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES} ico.o)
target_include_directories(${EXECUTABLE_NAME} PUBLIC src)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY})
    add_compile_definitions(NDEBUG)
else ()
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin/${CMAKE_BUILD_TYPE})
endif ()

find_package(SFML 2.5.1 REQUIRED graphics window system audio network)
target_link_libraries(${EXECUTABLE_NAME} sfml-system sfml-window sfml-graphics sfml-audio sfml-network)
target_link_libraries(${EXECUTABLE_NAME} stdc++fs)

