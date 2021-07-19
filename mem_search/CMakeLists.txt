if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(MemSearch "main.cpp" "mem_search.hpp" "win32.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(mem_search "main.cpp" "mem_search.hpp" "linux.cpp")
endif()
