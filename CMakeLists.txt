cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/W4 /WX)
	add_executable(RandFile "rand_file.cpp")
	add_executable(DirClean "dir_clean.cpp")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Wall -Wextra -pedantic -Werror)

	add_executable(rand_file "rand_file.cpp")
	target_link_libraries(rand_file stdc++fs)

	add_executable(dir_clean "dir_clean.cpp")
	target_link_libraries(dir_clean stdc++fs)
endif()