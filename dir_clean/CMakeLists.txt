if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(DirClean "dir_clean.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(dir_clean "dir_clean.cpp")
	target_link_libraries(dir_clean stdc++fs)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(dir_clean "dir_clean.cpp")
endif()
