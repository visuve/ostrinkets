cmake_minimum_required (VERSION 3.8)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_executable(RandFile "rand_file.cpp")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_executable(rand_file "rand_file.cpp")
	target_link_libraries(rand_file stdc++fs)
endif()