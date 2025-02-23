cmake_minimum_required (VERSION 3.12)

project (nve)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(BUILD_TESTS "WithTests" OFF)

set(SOURCES
	"src/Editor/Editor.cpp"
	"src/File/File.cpp"
	"src/Input/Input.cpp"
	"src/SyntaxHighlight/SyntaxHighlight.cpp"
	"src/main.cpp"
)

set (HEADERS
	"src/Console/Console.hpp"
	"src/Editor/Editor.hpp"
	"src/File/File.hpp"
	"src/Input/Input.hpp"
	"src/Input/GetInputImpl.hpp"
	"src/KeyActions/KeyActions.hh"
	"src/SyntaxHighlight/SyntaxHighlight.hpp"
)

add_executable (nve ${SOURCES} ${HEADERS})

if(WIN32)
	target_sources(nve
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Console/Windows/Console.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Input/Windows/GetInputImpl.cpp"
	)
else()
	target_sources(nve
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Console/Unix/Console.cpp"
		PRIVATE "${CMAKE_CURRENT_LIST_DIR}/src/Input/Unix/GetInputImpl.cpp"
	)
endif()



target_include_directories(nve PRIVATE src)

if(BUILD_TESTS)
	add_library(nve_project ${SOURCES} ${HEADERS})

	if(WIN32)
		target_sources(nve_project
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Console/Windows/Console.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Input/Windows/GetInputImpl.cpp"
		)
	else()
		target_sources(nve_project
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Console/Unix/Console.cpp"
			PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/Input/Unix/GetInputImpl.cpp"
		)
	endif(WIN32)

	target_include_directories(nve_project PUBLIC src)
	add_subdirectory(tests)
endif(BUILD_TESTS)
