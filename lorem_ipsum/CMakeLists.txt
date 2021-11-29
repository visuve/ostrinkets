if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(LOREM_IPSUM_SRC lorem_ipsum.c)
	set(LOREM_IPSUM_KMOD lorem_ipsum.ko)

	set(KERNEL_DIR "/lib/modules/${CMAKE_SYSTEM_VERSION}/build" )

	if(CMAKE_SYSTEM_VERSION MATCHES "Microsoft")
		set(KERNEL_DIR "/lib/modules/5.11.0-40-generic/build")
	endif()

	set(KBUILD_CMD make -C ${KERNEL_DIR} M=${CMAKE_CURRENT_SOURCE_DIR}  modules)

	add_custom_command(OUTPUT ${LOREM_IPSUM_KMOD} COMMAND ${KBUILD_CMD} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${LOREM_IPSUM_SRC} Kbuild VERBATIM)

	add_custom_target(lorem_ipsum ALL DEPENDS ${LOREM_IPSUM_KMOD})
endif()
