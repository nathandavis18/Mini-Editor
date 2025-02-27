#include <gtest/gtest.h>
#include "Console/Console.hpp"

TEST(ConsoleTests, ConsoleInitializesProperly)
{
	Console console;
	EXPECT_NO_FATAL_FAILURE();
}


TEST(ConsoleTests, ConsoleEnableAndDisableRawInputDoesntFail)
{
	Console console;
	console.enableRawInput();
	console.disableRawInput();
	EXPECT_NO_FATAL_FAILURE();
}

TEST(ConsoleTests, ConsoleSizeGetsSetProperly)
{
	Console console;
	Console::WindowSize size = console.getWindowSize();
	EXPECT_TRUE(size.rows > 0 && size.cols > 0);
}

TEST(ConsoleTests, ConsoleSizeHasChangedWorks)
{
	Console console;
	Console::WindowSize size = console.getWindowSize();
	bool withoutChange = console.windowSizeHasChanged(size.rows, size.cols);
	EXPECT_FALSE(withoutChange);

	size.rows -= 1;
	bool withChange = console.windowSizeHasChanged(size.rows, size.cols);
	EXPECT_TRUE(withChange);
}