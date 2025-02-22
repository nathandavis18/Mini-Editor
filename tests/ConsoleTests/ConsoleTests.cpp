#include <gtest/gtest.h>
#include "Console/Console.hpp"

TEST(ConsoleTests, ConsoleInitializesProperly)
{
	Console::initConsole();
	EXPECT_NO_FATAL_FAILURE();
}


TEST(ConsoleTests, ConsoleEnableAndDisableRawInputDoesntFail)
{
	Console::disableRawInput();
	Console::enableRawInput();
	EXPECT_NO_FATAL_FAILURE();
}

TEST(ConsoleTests, ConsoleSizeGetsSetProperly)
{
	Console::WindowSize size = Console::getWindowSize();
	EXPECT_TRUE(size.rows > 0 && size.cols > 0);
}

TEST(ConsoleTests, ConsoleSizeHasChangedWorks)
{
	Console::WindowSize size = Console::getWindowSize();
	bool withoutChange = Console::windowSizeHasChanged(size.rows, size.cols);
	EXPECT_FALSE(withoutChange);

	size.rows -= 1;
	bool withChange = Console::windowSizeHasChanged(size.rows, size.cols);
	EXPECT_TRUE(withChange);
}