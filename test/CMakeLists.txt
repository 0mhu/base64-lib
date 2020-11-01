project(base64-test)

add_custom_target(test "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}" "-r compact" "-s" DEPENDS ${PROJECT_NAME})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/catch-framework")
aux_source_directory("src" TEST_SOURCES)

add_executable(${PROJECT_NAME} ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME} base64-lib)

