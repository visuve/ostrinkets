if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(BytePrinter "main.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(byte_printer "main.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(byte_printer"main.cpp")
endif()
