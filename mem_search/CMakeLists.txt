if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(mem_search "main.cpp" "mem_search.hpp" "linux.cpp")
endif()
