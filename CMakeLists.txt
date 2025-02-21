cmake_minimum_required (VERSION 3.12)

project (nve)

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
		PRIVATE "src/Console/Windows/Console.cpp"
		PRIVATE "src/Input/Windows/GetInputImpl.cpp"
	)
else()
	target_sources(nve
		PRIVATE "src/Console/Unix/Console.cpp"
		PRIVATE "src/Input/Unix/GetInputImpl.cpp"
	)
endif()

target_include_directories(nve PRIVATE src)

set_property(TARGET nve PROPERTY CXX_STANDARD 20)
