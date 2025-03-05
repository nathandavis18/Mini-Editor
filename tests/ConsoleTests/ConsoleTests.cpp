#include <gtest/gtest.h>
#include "Console/Console.hpp"

TEST(ConsoleTests, ConsoleInitializesProperly)
{
	EXPECT_NO_FATAL_FAILURE(Console console);
}


TEST(ConsoleTests, ConsoleEnableAndDisableRawInputDoesntFail)
{
	EXPECT_NO_FATAL_FAILURE(
		Console console,
		console.enableRawInput(),
		console.disableRawInput()
	);
}

TEST(ConsoleTests, ConsoleSizeGetsSetProperly)
{
	Console console;
	Console::WindowSize size = console.getWindowSize();
	EXPECT_TRUE(size.rows > 0 && size.cols > 0);
}