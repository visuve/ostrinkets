if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(LOREM_IPSUM_SRC
		"${CMAKE_CURRENT_SOURCE_DIR}/lorem_ipsum.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/Kbuild"
	)

	set(LOREM_IPSUM_KMOD "${CMAKE_CURRENT_BINARY_DIR}/lorem_ipsum.ko")

	message(${CMAKE_SYSTEM_VERSION})

	if(CMAKE_SYSTEM_VERSION MATCHES "microsoft")
		set(KERNEL_DIR "/lib/modules/5.15.0-46-generic/build" CACHE PATH "Kernel headers" FORCE)
		if (NOT EXISTS ${KERNEL_DIR})
			message(WARNING "Kernel headers at ${KERNEL_DIR} not found. Did you remember to install linux-headers-generic?")
		endif()
	else()
		set(KERNEL_DIR "/lib/modules/${CMAKE_SYSTEM_VERSION}/build" CACHE PATH "Kernel headers" FORCE)
	endif()

	message("${CMAKE_CURRENT_BINARY_DIR}")
	message("${KERNEL_DIR}")

	set(KBUILD_CMD make -C ${KERNEL_DIR} M=${CMAKE_CURRENT_BINARY_DIR})

	add_custom_target(LOREM_IPSUM_COPY ALL COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LOREM_IPSUM_SRC} ${CMAKE_CURRENT_BINARY_DIR})

	add_custom_command(OUTPUT ${LOREM_IPSUM_KMOD} COMMAND ${KBUILD_CMD} WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} DEPENDS ${LOREM_IPSUM_COPY} Kbuild VERBATIM)

	add_custom_target(lorem_ipsum ALL DEPENDS ${LOREM_IPSUM_KMOD})
endif()
