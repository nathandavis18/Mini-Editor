cmake_minimum_required (VERSION 3.14)

project (mini)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(BUILD_TESTS "WithTests" OFF)
option(BUILD_PROJECT "BuildMini" ON)
option(CREATE_RELEASE "CpackRelease" OFF)
option(DEBUG_BUILD "Debug" OFF)


set(SOURCES
	"src/Editor/Editor.cpp"
	"src/File/File.cpp"
	"src/Input/Input.cpp"
	"src/SyntaxHighlight/SyntaxHighlight.cpp"
	"src/Utility/JsonParser/JsonParser.cpp"
	"src/FindAndReplace/FindAndReplace.cpp"
	"src/Renderer/Renderer.cpp"
)

set (HEADERS
	"src/Console/Console.hpp"
	"src/Editor/Editor.hpp"
	"src/File/File.hpp"
	"src/Input/Input.hpp"
	"src/Input/InputImpl.hpp"
	"src/KeyActions/KeyActions.hh"
	"src/SyntaxHighlight/SyntaxHighlight.hpp"
	"src/Utility/GetProgramPath/GetProgramPath.hpp"
	"src/Utility/JsonParser/JsonParser.hpp"
	"src/EventHandler/EventHandler.hpp"
	"src/FindAndReplace/FindAndReplace.hpp"
	"src/Renderer/Renderer.hpp"
)

if(BUILD_PROJECT)
	if(DEBUG_BUILD)
		add_compile_definitions(DEBUG_BUILD)
	endif(DEBUG_BUILD)
	add_executable (mini "src/main.cpp" ${SOURCES} ${HEADERS})

	if(WIN32)
		target_sources(mini
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Console/Windows/Console.cpp"
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Input/Windows/InputImpl.cpp"
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Windows/EventHandler.cpp"
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Utility/GetProgramPath/Windows/GetProgramPath.cpp"
		)
	else()
		target_sources(mini
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Console/Unix/Console.cpp"
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Input/Unix/InputImpl.cpp"
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Unix/EventHandler.cpp"
			PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Utility/GetProgramPath/Unix/GetProgramPath.cpp"
		)
	endif()

	add_custom_command(
		TARGET mini
		COMMENT "Copying Syntax File"
		PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/config.json $<TARGET_FILE_DIR:mini>
		VERBATIM
	)

	target_include_directories(mini PRIVATE src)
endif(BUILD_PROJECT)

if(BUILD_TESTS)
	add_compile_definitions(TESTING)

	add_library(mini_tests ${SOURCES} ${HEADERS})

	if(WIN32)
		target_sources(mini_tests
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Console/Windows/Console.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Input/Windows/InputImpl.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Windows/EventHandler.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Utility/GetProgramPath/Windows/GetProgramPath.cpp"
		)
	else()
		target_sources(mini_tests
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Console/Unix/Console.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Input/Unix/InputImpl.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Unix/EventHandler.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Utility/GetProgramPath/Unix/GetProgramPath.cpp"
		)
	endif(WIN32)

	target_include_directories(mini_tests PUBLIC src)
	add_subdirectory(tests)
endif(BUILD_TESTS)

if(CREATE_RELEASE)
	install(TARGETS mini DESTINATION /)
	install(FILES "${CMAKE_CURRENT_LIST_DIR}/config.json" DESTINATION /)
	set(CPACK_PACKAGE_NAME "mini")
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
	set(CPACK_PACKAGE_VERSION "0.7.0a")
	set(CPACK_PACKAGE_VERSION_MAJOR "0")
	set(CPACK_PACKAGE_VERSION_MINOR "7")
	set(CPACK_PACKAGE_VERSION_PATCH "0")
	set(CPACK_GENERATOR "ZIP")
	set(CPACK_SOURCE_GENERATOR "ZIP")
	set(CPACK_PACKAGE_FILE_NAME "mini")
	include(CPack)
endif(CREATE_RELEASE)