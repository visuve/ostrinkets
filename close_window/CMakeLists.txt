if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(close_window "close_window.cpp")
	target_link_libraries(close_window X11)
endif()