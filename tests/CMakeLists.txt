cmake_minimum_required (VERSION 3.12)

project(Tests)

include(FetchContent)
FetchContent_Declare(
	googletest
	GIT_REPOSITORY https://github.com/google/googletest
	GIT_TAG v1.16.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

set(SOURCES
	"testsMain.cpp"
	"ConsoleTests/ConsoleTests.cpp"
	"EditorTests/EditorTests.cpp"
	"FileTests/FileTests.cpp"
	"InputTests/InputTests.cpp"
	"SyntaxHighlightTests/SyntaxHighlightTests.cpp"
	"JsonParserTests/JsonParserTests.cpp"
	"GetProgramPathTests/GetProgramPathTests.cpp"
	"FindStringTests/FindStringTests.cpp"
	"RendererTests/RendererTests.cpp"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(Tests ${SOURCES} "MockClasses/MockConsole.hpp")
target_link_libraries(Tests PUBLIC
	gtest_main
	mini_tests
)
target_include_directories(Tests PRIVATE MockClasses)

add_test(
	NAME Tests
	COMMAND Tests
)

add_custom_command(
	TARGET Tests
	COMMENT "Copying testFile.txt"
	PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/testFile.txt $<TARGET_FILE_DIR:Tests>
	VERBATIM
)

add_custom_command(
	TARGET Tests
	COMMENT "Copying config.json"
	PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/config.json $<TARGET_FILE_DIR:Tests>
	VERBATIM
)

add_custom_command(
	TARGET Tests
	COMMENT "Copying testJson.json"
	PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/testJson.json $<TARGET_FILE_DIR:Tests>
	VERBATIM
)

add_custom_command(
	TARGET Tests
	COMMENT "Copying testJson.json"
	PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/test.cpp $<TARGET_FILE_DIR:Tests>
	VERBATIM
)