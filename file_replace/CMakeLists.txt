if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(FileReplace "file_replace.cpp" "memory_mapped_file_win32.cpp")
endif()
