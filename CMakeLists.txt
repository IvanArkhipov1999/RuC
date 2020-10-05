cmake_minimum_required(VERSION 3.0.0)

project(ruc)


option(ANALYSIS_ENABLED "Enable static analysis of input files" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Put all libraries to one folder
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
	add_compile_options(-Werror=return-type -g -std=c99)
endif()

if(ANALYSIS_ENABLED)
	include(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/aspsimple.cmake)
endif()


# Add libraries
add_subdirectory(libs)

# Add frontend
add_subdirectory(src)


# Set install parameters
install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/ruc)
