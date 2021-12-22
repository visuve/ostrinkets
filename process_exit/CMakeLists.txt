if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(ProcessExit "main.cpp" "process_win32.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(process_exit "main.cpp" "process_posix.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(process_exit "main.cpp" "process_posix.cpp")
endif()
