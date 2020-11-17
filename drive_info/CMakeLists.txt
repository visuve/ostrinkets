if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_executable(DriveInfo "main.cpp" "drive_info_win.cpp")
	target_link_libraries(DriveInfo wbemuuid)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	add_executable(drive_info "main.cpp" "drive_info_linux.cpp")
	target_link_libraries(drive_info stdc++fs udev)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	add_executable(drive_info "main.cpp" "drive_info_bsd.cpp")
	target_link_libraries(drive_info geom)
endif()
