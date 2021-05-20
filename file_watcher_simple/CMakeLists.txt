if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(FileWatcherSimple "main.cpp" )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(file_watcher_simple "main.cpp")
	target_link_libraries(file_watcher_simple)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(file_watcher_simple "main.cpp")
endif()
