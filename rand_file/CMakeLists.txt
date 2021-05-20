if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(RandFile "rand_file.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(rand_file "rand_file.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(rand_file "rand_file.cpp")
endif()
