if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(FileWatcher "main.cpp" )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(file_watcher "main.cpp")
	target_link_libraries(file_watcher stdc++fs)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(file_watcher "main.cpp")
endif()
