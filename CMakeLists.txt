cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

project("fstrinkets")

if(MSVC)
	add_compile_options(/W4 /WX /MP)
else()
	add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory("dir_clean")
add_subdirectory("rand_file")
add_subdirectory("drive_info")
add_subdirectory("sloth_copy")
add_subdirectory("entropy_calc")
add_subdirectory("file_info")
add_subdirectory("bit_flipper")
