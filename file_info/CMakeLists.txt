if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(FileInfo "main.cpp" "file_info_win32.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(file_info "main.cpp" "file_info_posix.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(file_info "main.cpp" "file_info_posix.cpp")
endif()
