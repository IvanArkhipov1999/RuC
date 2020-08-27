# Find the compiler
find_program(
	CMAKE_RUC_COMPILER
		NAMES "/home/victor/ruc/build/ruc" 
		HINTS "${CMAKE_SOURCE_DIR}"
		DOC "RuC compiler"
)
mark_as_advanced(CMAKE_RUC_COMPILER)

set(CMAKE_RUC_SOURCE_FILE_EXTENSIONS c;C)
set(CMAKE_RUC_OUTPUT_EXTENSION)
set(CMAKE_RUC_COMPILER_ENV_VAR "RUC")

# Configure variables set in this file for fast reload later on
configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeRUCCompiler.cmake.in
				${CMAKE_PLATFORM_INFO_DIR}/CMakeRUCCompiler.cmake)
