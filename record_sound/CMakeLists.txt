if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(RecordSound "main.cpp" "record_sound_win32.cpp")
endif()
