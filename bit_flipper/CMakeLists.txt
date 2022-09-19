if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(BitFlipper "main.cpp" "win32.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux|FreeBSD")
	add_executable(bit_flipper "main.cpp" "posix.cpp")
endif()
