cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project("fstrinkets")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/W4 /WX)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory("dir_clean")
add_subdirectory("rand_file")
add_subdirectory("drive_info")