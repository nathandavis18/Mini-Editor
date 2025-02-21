/**
* MIT License

Copyright (c) 2024 Nathan Davis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Console/Console.hpp"

#include <Windows.h>
#include <iostream>

namespace Console
{
	DWORD defaultMode; //Windows Console settings is stored in DWORD (an unsigned long)
	bool rawModeEnabled = false;
	WindowSize windowSize;

	void initConsole()
	{
		detail::setDefaultMode();
		detail::setWindowSize();
		if (!enableRawInput())
		{
			std::cerr << "Error enabling raw input mode";
			exit(EXIT_FAILURE);
		}
	}

	void detail::setDefaultMode()
	{
		if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode)) //Try to get the default terminal settings
		{
			std::cerr << "Error retrieving current console mode";
			exit(EXIT_FAILURE);
		}
	}

	WindowSize getWindowSize()
	{
		return windowSize;
	}

	void detail::setWindowSize()
	{
		CONSOLE_SCREEN_BUFFER_INFO screenInfo; //Windows console size struct
		if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenInfo))
		{
			std::cerr << "Error getting console screen buffer info";
			exit(EXIT_FAILURE);
		}

		windowSize.rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1;
		windowSize.cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;
	}

	bool windowSizeHasChanged(const int prevRows, const int prevCols)
	{
		detail::setWindowSize();
		if (prevRows != windowSize.rows || prevCols != windowSize.cols)
		{
			return true;
		}
		return false;
	}

	bool enableRawInput()
	{
		if (rawModeEnabled) return true;

		DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT
			& ~ENABLE_ECHO_INPUT & ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT); //Disabling certain input/output flags to enable raw mode

		if (SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode))
		{
			atexit(disableRawInput); //Make sure raw input mode gets disabled if the program exits due to an error
			rawModeEnabled = true;
		}
		return rawModeEnabled;
	}

	void disableRawInput()
	{
		SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
		rawModeEnabled = false;
	}
}