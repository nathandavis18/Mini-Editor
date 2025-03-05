#pragma once
#include "Console/Console.hpp"

class MockConsole : public Console
{
public:
	MockConsole() : windowSize(WindowSize(10, 10))
	{}
	~MockConsole()
	{
		return;
	}
	void enableRawInput() override
	{
		return;
	}
	void disableRawInput() override
	{
		return;
	}
	WindowSize getWindowSize() override
	{
		return windowSize;
	}
private:
	WindowSize windowSize;
};