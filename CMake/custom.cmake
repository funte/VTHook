# Check if we're compiling with MSVC
if(NOT MSVC)
	message(FATAL_ERROR "Non-MSVC compilers are not supported!")
endif()

# Fail when trying to compile to a path that contains spaces
string(FIND "${PROJECT_BINARY_DIR}" " " SPACE_INDEX)
if(NOT SPACE_INDEX STREQUAL "-1")
	message(FATAL_ERROR "Compiling in a path that contains spaces is not supported!")
endif()
