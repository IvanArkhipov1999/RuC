cmake_minimum_required(VERSION 2.8)

project(cmake-include)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
enable_language(RUC)

file(GLOB SRCS "*.c")
#add_library(${PROJECT_NAME} OBJECT ${SRCS})

add_executable(${PROJECT_NAME} ${SRCS})
