if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(RandFile "rand_file.cpp")
else()
	add_executable(rand_file "rand_file.cpp")
endif()
