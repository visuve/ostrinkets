cmake_minimum_required (VERSION 3.8)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(DriveInfo "main.cpp" "drive_info_win.cpp")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(drive_info "main.cpp" "drive_info_linux.cpp")
	target_link_libraries(drive_info stdc++fs)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(drive_info "main.cpp" "drive_info_bsd.cpp")
	target_link_libraries(drive_info LINK_PRIVATE geom)
endif()