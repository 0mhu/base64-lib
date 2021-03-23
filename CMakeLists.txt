cmake_minimum_required(VERSION 3.0)
project(base64-lib)

aux_source_directory("src" SOURCES)
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(test)
