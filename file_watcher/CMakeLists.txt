if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(FileWatcher "main.cpp" "file_watcher_win32.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(file_watcher "main.cpp" "file_watcher_linux.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(file_watcher "main.cpp" "file_watcher_bsd.cpp")
endif()
