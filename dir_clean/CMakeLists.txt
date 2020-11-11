cmake_minimum_required (VERSION 3.8)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_executable(DirClean "dir_clean.cpp")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_executable(dir_clean "dir_clean.cpp")
	target_link_libraries(dir_clean stdc++fs)
endif()