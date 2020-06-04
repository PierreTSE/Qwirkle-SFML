cmake_minimum_required(VERSION 3.0)
project(Qwirkle-SFML)

set(EXECUTABLE_NAME "Qwirkle")

set(CMAKE_CXX_STANDARD 17)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Os -Wall -Wextra -Wno-switch")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os")
endif ()


file(GLOB_RECURSE SOURCE_FILES src/*)
add_executable(${EXECUTABLE_NAME} ${SOURCE_FILES} ico.o)
target_include_directories(${EXECUTABLE_NAME} PUBLIC src/Engine)

if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY})
else ()
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_HOME_DIRECTORY}/bin/${CMAKE_BUILD_TYPE})
endif ()

find_package(SFML 2 REQUIRED graphics window system audio)
target_link_libraries(${EXECUTABLE_NAME} sfml-system sfml-window sfml-graphics sfml-audio)
target_link_libraries(${EXECUTABLE_NAME} stdc++fs)

