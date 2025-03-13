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

#include "Input/Input.hpp"
#include "Editor/Editor.hpp"
#include "EventHandler/EventHandler.hpp"
#include "KeyActions/KeyActions.hh"
#include "File/File.hpp"
#include "Console/Console.hpp"

#include <iostream>
#include <string_view>
#include <atomic>
#include <cstdlib> //EXIT_FAILURE, EXIT_SUCCESS

int main(int argc, const char** argv)
{
#ifndef NDEBUG
	argc = 2;
	argv[1] = "test.cpp";
#endif
	if (argc != 2)
	{
		std::cerr << "ERROR: Usage: mini <filename>\n";
		return EXIT_FAILURE;
	}

	std::string_view fName = argv[1];
	std::string_view extension;
	size_t extensionIndex;
	if ((extensionIndex = fName.find_last_of('.')) != std::string_view::npos)
	{
		extension = fName.substr(extensionIndex);
	}
	else
	{
		extension = std::string_view();
	}

	Editor editor(SyntaxHighlight(extension), FileHandler(fName), std::make_unique<Console>(Console()));

	std::atomic<bool> running = true;
	EventHandler evtHandler(running, &editor);

	while (true)
	{
		if (editor.mode() == Editor::Mode::ExitMode)
		{
			running = false;
			editor.clearScreen();
			break;
		}
		else
		{
			editor.refreshScreen();
			const KeyActions::KeyAction inputCode = InputHandler::getInput();
			if (inputCode != KeyActions::KeyAction::None)
			{
				if (editor.mode() == Editor::Mode::CommandMode || editor.mode() == Editor::Mode::ReadMode)
				{
					InputHandler::changeMode(inputCode, editor);
				}
				else if (editor.mode() == Editor::Mode::EditMode)
				{
					InputHandler::handleInput(inputCode, editor);
				}
				else if (editor.mode() == Editor::Mode::FindMode)
				{
					InputHandler::findModeInput(inputCode, editor);
				}
			}
		}
	}
	
	return EXIT_SUCCESS;
}