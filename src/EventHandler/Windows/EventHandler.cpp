/**
* MIT License

Copyright (c) 2025 Nathan Davis

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

#include "EventHandler/EventHandler.hpp"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include <thread>

std::thread t;
Editor* editor = nullptr;

/// <summary>
/// Handles checking and updating the window size using a blocking call on a secondary thread to avoid busy looping
/// If the Console Input is a user input, put it back into the queue for _getch() to retrieve
/// </summary>
/// <param name="running"></param>
void windowSizeChangeEvent(std::atomic<bool>& running)
{
	while (running)
	{
		INPUT_RECORD input;
		DWORD numEvents;

		ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &numEvents); //Blocks this thread until an event happens
		if (input.EventType == WINDOW_BUFFER_SIZE_EVENT) //If the event is a window size update
		{
			editor->updateWindowSize();
			editor->refreshScreen(true);
		}
		else
		{
			DWORD _; //Can be ignored, but WriteConsoleInput requires it
			WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 1, &_);
		}
	}
}

EventHandler::EventHandler(std::atomic<bool>& running, Editor* ed)
{
	editor = ed;
	t = std::thread(windowSizeChangeEvent, std::ref(running));
}

EventHandler::~EventHandler()
{
	t.join();
}