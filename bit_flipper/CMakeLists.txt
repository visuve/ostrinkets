if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(BitFlipper "main.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(bit_flipper "main.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(bit_flipper "main.cpp")
endif()
