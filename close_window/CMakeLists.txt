if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(CloseWindow "main.cpp" "window_manager_win32.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(close_window "main.cpp" "window_manager_x11.cpp")
	target_link_libraries(close_window X11)
endif()
