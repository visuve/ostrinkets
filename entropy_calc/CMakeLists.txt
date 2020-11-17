if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(EntropyCalc "entropy_calc.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(entropy_calc "entropy_calc.cpp")
	target_link_libraries(entropy_calc stdc++fs)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(entropy_calc "entropy_calc.cpp")
endif()
