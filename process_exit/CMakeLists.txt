if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(ProcessExit "main.cpp" "process_win32.cpp")
else()
	add_executable(process_exit "main.cpp" "process_posix.cpp")
endif()
