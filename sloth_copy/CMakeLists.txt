if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(SlothCopy "sloth_copy.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(sloth_copy "sloth_copy.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(sloth_copy "sloth_copy.cpp")
endif()
