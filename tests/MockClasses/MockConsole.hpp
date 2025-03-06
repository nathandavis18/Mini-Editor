#pragma once
#include "Console/ConsoleInterface.hpp"

class MockConsole : public IConsole
{
public:
	MockConsole() : mWindowSize(WindowSize(10, 10))
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
		return mWindowSize;
	}
private:
	void setDefaultMode() override
	{
		return;
	}
	void setWindowSize() override
	{
		return;
	}
private:
	WindowSize mWindowSize;
};