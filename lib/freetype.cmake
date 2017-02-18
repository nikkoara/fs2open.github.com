
add_library(freetype INTERFACE)

if(PLATFORM_WINDOWS)
	# We use prebuilt binaries for windows
	if (IS_64BIT)
		set(SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/freetype/Win64")
	else()
		set(SEARCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/freetype/Win32")
	endif()

	set(dll_name "${SEARCH_PATH}/freetype281.dll")

	target_include_directories(freetype INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/freetype/include")

	target_link_libraries(freetype INTERFACE "${SEARCH_PATH}/${CMAKE_IMPORT_LIBRARY_PREFIX}freetype281${CMAKE_IMPORT_LIBRARY_SUFFIX}")

	add_target_copy_files("${dll_name}")
else()
	find_package(Freetype REQUIRED)

	target_include_directories(freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
	target_link_libraries(freetype INTERFACE ${FREETYPE_LIBRARIES})
endif()
