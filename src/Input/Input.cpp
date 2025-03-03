﻿/**
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

#include "Input.hpp"
#include "InputImpl.hpp"

#include <iostream>
#include <string>

using KeyActions::KeyAction;

namespace InputHandler
{
	const KeyAction getInput()
	{
		return InputImpl::getInput();
	}

	void changeMode(const KeyAction key, Editor& editor)
	{
		bool shouldExit = false;
		switch (key)
		{
		case static_cast<KeyAction>('i'):
			editor.enableEditMode();
			break;
		case static_cast<KeyAction>(':'):
			editor.enableCommandMode();
			editor.refreshScreen();

			shouldExit = doCommand(editor);
			editor.updateCommandBuffer(std::string());

			if (!shouldExit)
			{
				editor.enableReadMode(); //Go back to read mode after executing a command
				editor.refreshScreen(true);
			}
			break;

		case KeyAction::ArrowDown:
		case KeyAction::ArrowUp:
		case KeyAction::ArrowLeft:
		case KeyAction::ArrowRight:
		case KeyAction::CtrlArrowLeft:
		case KeyAction::CtrlArrowRight:
		case KeyAction::Home:
		case KeyAction::End:
		case KeyAction::CtrlHome:
		case KeyAction::CtrlEnd:
		case KeyAction::CtrlPageDown:
		case KeyAction::CtrlPageUp:
			editor.moveCursor(key);
			break;

		case KeyAction::CtrlArrowDown:
		case KeyAction::CtrlArrowUp:
		case KeyAction::PageDown:
		case KeyAction::PageUp:
			editor.shiftRowOffset(key);
			break;

		default: //Unknown command. Just go back to read mode
			editor.enableReadMode();
			break;
		}
	}

	void handleInput(KeyAction key, Editor& editor)
	{
		switch (key)
		{
		case KeyAction::Esc:
			editor.enableReadMode();
			break;
		case KeyAction::Delete:
		case KeyAction::Backspace:
		case KeyAction::CtrlBackspace:
		case KeyAction::CtrlDelete:
			editor.deleteChar(key);
			break;
		case KeyAction::ArrowDown:
		case KeyAction::ArrowUp:
		case KeyAction::ArrowLeft:
		case KeyAction::ArrowRight:
		case KeyAction::CtrlArrowLeft:
		case KeyAction::CtrlArrowRight:
		case KeyAction::Home:
		case KeyAction::End:
		case KeyAction::CtrlHome:
		case KeyAction::CtrlEnd:
		case KeyAction::PageDown:
		case KeyAction::PageUp:
		case KeyAction::CtrlPageDown:
		case KeyAction::CtrlPageUp:
			editor.moveCursor(key);
			break;
		case KeyAction::CtrlArrowDown:
		case KeyAction::CtrlArrowUp:
			editor.shiftRowOffset(key);
			break;
		case KeyAction::Enter:
			editor.addRow();
			break;
		case KeyAction::CtrlZ:
			editor.undoChange();
			break;
		case KeyAction::CtrlY:
			editor.redoChange();
			break;
		case KeyAction::CtrlC: //Don't need to do anything for this yet
			break;

		default: [[ likely ]] //This is the most likely scenario
			editor.insertChar(static_cast<uint8_t>(key));
			break;
		}
	}

	bool isActionKey(const KeyAction key)
	{
		switch (key)
		{
		case KeyAction::ArrowDown:
		case KeyAction::ArrowLeft:
		case KeyAction::ArrowRight:
		case KeyAction::ArrowUp:
		case KeyAction::CtrlArrowDown:
		case KeyAction::CtrlArrowLeft:
		case KeyAction::CtrlArrowRight:
		case KeyAction::CtrlArrowUp:
		case KeyAction::Home:
		case KeyAction::End:
		case KeyAction::CtrlHome:
		case KeyAction::CtrlEnd:
		case KeyAction::PageDown:
		case KeyAction::PageUp:
		case KeyAction::CtrlPageDown:
		case KeyAction::CtrlPageUp:
		case KeyAction::CtrlC:
		case KeyAction::CtrlX:
		case KeyAction::CtrlY:
		case KeyAction::CtrlZ:
		case KeyAction::Delete:
		case KeyAction::CtrlDelete:
		case KeyAction::Tab:
			return true;

		default:
			return false;
		}
	}

	bool doCommand(Editor& editor)
	{
		bool shouldExit = false;
		const char* startStr = "\r\x1b[0K:";
		std::string command;
		std::string commandBuffer = startStr;
		KeyAction commandInput;
		std::cout << commandBuffer;
		std::cout.flush();
		editor.updateCommandBuffer(commandBuffer);
		while ((commandInput = getInput()) != KeyAction::Enter)
		{
			if (!isActionKey(commandInput))
			{
				if (commandInput == KeyAction::Esc) return shouldExit;
				if (commandInput == KeyAction::Backspace && command.length() > 0)
				{
					command.pop_back();
				}
				else if (commandInput == KeyAction::CtrlBackspace)
				{
					command.clear();
				}
				else if (commandInput != KeyAction::CtrlBackspace && commandInput != KeyAction::Backspace)
				{
					command += static_cast<unsigned char>(commandInput);
				}
			}
			commandBuffer = startStr + command;
			editor.updateCommandBuffer(commandBuffer);
			std::cout << commandBuffer;
			std::cout.flush();
		}

		if ((command == "q" && !editor.isDirty()) || command == "q!") //Quit command - requires changes to be saved if not force quit
		{
			editor.enableExitMode();
			shouldExit = true;
		}
		else if (command == "w" || command == "s") //Save commands ([w]rite / [s]ave)
		{
			editor.save();
		}
		else if (command == "wq" || command == "sq") //Save and quit commands ([w]rite [q]uit / [s]ave [q]uit)
		{
			editor.save();
			editor.enableExitMode();
			shouldExit = true;
		}
		return shouldExit;
	}
}
