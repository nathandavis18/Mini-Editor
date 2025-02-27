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

#include "Input/Input.hpp"
#include "Editor/Editor.hpp"
#include "Console/Console.hpp"

#include <iostream>
#include <thread>
#include <cstdlib>

/// <summary>
/// Makes sure that if the user changes the size of the console, the screen updates accordingly
/// </summary>
/// <param name="runThread"></param>
void updateScreen(std::atomic<bool>& runThread)
{
	while (runThread)
	{
		if (Editor::windowSizeHasChanged()) //Only update if the terminal screen actually got updated
		{
			Editor::updateWindowSize();
			Editor::refreshScreen(true);
		}
	}
}

int main(int argc, const char** argv)
{
#ifndef NDEBUG
	argc = 2;
	argv[1] = "test.cpp";
#endif
	if (argc != 2)
	{
		std::cerr << "ERROR: Usage: nve <filename>\n";
		return EXIT_FAILURE;
	}

	Editor::initEditor(argv[1]);

	std::atomic<bool> runThread = true; //Main thread will update this bool, while secondary thread reads from it
	std::thread t(updateScreen, std::ref(runThread));

	while (true)
	{
		if (Editor::mode() == Editor::Mode::ExitMode)
		{
			runThread = false;
			while (!t.joinable()); //Wait for secondary thread to finish what it is doing, then join the thread
			t.join();

			Editor::clearScreen();
			break;
		}
		else
		{
			Editor::refreshScreen();
			const KeyActions::KeyAction inputCode = InputHandler::getInput();
			if (inputCode != KeyActions::KeyAction::None)
			{
				if (Editor::mode() == Editor::Mode::CommandMode || Editor::mode() == Editor::Mode::ReadMode)
				{
					InputHandler::doCommand(inputCode);
				}
				else if (Editor::mode() == Editor::Mode::EditMode)
				{
					InputHandler::handleInput(inputCode);
				}
			}
		}
	}
	
	return EXIT_SUCCESS;
}