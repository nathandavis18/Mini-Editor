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
#include "KeyActions/KeyActions.hh"

#include <string>
#include <iostream>

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>

namespace Console
{
	static termios defaultMode; //Unix console settings struct
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
		if (tcgetattr(STDOUT_FILENO, &defaultMode) == -1) //If the settings can't be retrieved
		{
			std::cerr << "Error retrieving current console mode";
			exit(EXIT_FAILURE);
		}
		signal(SIGWINCH, nullptr);
	}

	WindowSize getWindowSize()
	{
		return windowSize;
	}

	void detail::setWindowSize()
	{
		winsize ws; //Unix console size struct
		int rows, cols;

		WindowSize windowSize;
		ioctl(fileno(stdout), TIOCGWINSZ, &ws); //Get the console size from the OS
		windowSize.cols = ws.ws_col;
		windowSize.rows = ws.ws_row;
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

		termios raw;

		//Disabling some console processing flags
		raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
		raw.c_oflag &= ~(OPOST);
		raw.c_cflag |= (CS8);
		raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

		//Setting the timeout timer for read() so the screen doesn't appear frozen when getting input from user
		raw.c_cc[VMIN] = 0;
		raw.c_cc[VTIME] = 1;

		if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw) < 0) //If setting raw mode fails
		{
			return rawModeEnabled; //Setting raw mode failed, so return the current status
		}
		else
		{
			atexit(disableRawInput); //Make sure raw input mode gets disabled if the program exits due to an error
			rawModeEnabled = true;
		}

		return rawModeEnabled;
	}

	void disableRawInput()
	{
		tcsetattr(STDOUT_FILENO, TCSAFLUSH, &defaultMode);
		rawModeEnabled = false;
	}
}