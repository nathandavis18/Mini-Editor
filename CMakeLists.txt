cmake_minimum_required (VERSION 3.12)

project (mini)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(BUILD_TESTS "WithTests" OFF)

set(SOURCES
	"src/Editor/Editor.cpp"
	"src/File/File.cpp"
	"src/Input/Input.cpp"
	"src/SyntaxHighlight/SyntaxHighlight.cpp"
	"src/SyntaxHighlight/JsonParser/JsonParser.cpp"
	"src/main.cpp"
)

set (HEADERS
	"src/Console/Console.hpp"
	"src/Editor/Editor.hpp"
	"src/File/File.hpp"
	"src/Input/Input.hpp"
	"src/Input/InputImpl.hpp"
	"src/KeyActions/KeyActions.hh"
	"src/SyntaxHighlight/SyntaxHighlight.hpp"
	"src/SyntaxHighlight/GetProgramPath/GetProgramPath.hpp"
	"src/SyntaxHighlight/JsonParser/JsonParser.hpp"
	"src/EventHandler/EventHandler.hpp"
)

add_executable (mini ${SOURCES} ${HEADERS})

if(WIN32)
	target_sources(mini
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Console/Windows/Console.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Input/Windows/InputImpl.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Windows/EventHandler.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/SyntaxHighlight/GetProgramPath/Windows/GetProgramPath.cpp"
	)
else()
	target_sources(mini
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Console/Unix/Console.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Input/Unix/InputImpl.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Unix/EventHandler.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/SyntaxHighlight/GetProgramPath/Unix/GetProgramPath.cpp"
	)
endif()

add_custom_command(
	TARGET mini
	COMMENT "Copying Syntax File"
	PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/config.json $<TARGET_FILE_DIR:mini>
	VERBATIM
)

target_include_directories(mini PRIVATE src)

if(BUILD_TESTS)
	add_library(mini_tests ${SOURCES} ${HEADERS})

	if(WIN32)
		target_sources(mini_tests
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Console/Windows/Console.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Input/Windows/InputImpl.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Windows/EventHandler.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/SyntaxHighlight/GetProgramPath/Windows/GetProgramPath.cpp"
		)
	else()
		target_sources(mini_tests
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Console/Unix/Console.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Input/Unix/InputImpl.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/EventHandler/Unix/EventHandler.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/SyntaxHighlight/GetProgramPath/Unix/GetProgramPath.cpp"
		)
	endif(WIN32)

	target_include_directories(mini_tests PUBLIC src)
	add_subdirectory(tests)
endif(BUILD_TESTS)
