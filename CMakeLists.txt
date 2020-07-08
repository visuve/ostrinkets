cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_executable (RandFile Main.cpp)
	target_compile_options(RandFile PRIVATE /W4 /WX)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_executable (rand_file Main.cpp)
	target_compile_options(rand_file PRIVATE -Wall -Wextra -pedantic -Werror)
	target_link_libraries(rand_file stdc++fs)
endif()