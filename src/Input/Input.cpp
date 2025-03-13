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

#include "Input.hpp"
#include "InputImpl.hpp"

#include <iostream>
#include <string>

using KeyActions::KeyAction;

namespace InputHandler
{
	std::string previousFindString = "";

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

		case KeyAction::CtrlF:
		case static_cast<KeyAction>('f'):
			editor.enableFindMode();
			editor.refreshScreen();

			find(editor);
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

		case KeyAction::CtrlS:
			editor.save();
			break;

		case KeyAction::CtrlQ:
			editor.enableExitMode();
			break;

		default: //Unknown command. Just go back to read mode
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

		case KeyAction::CtrlX:
		case KeyAction::CtrlC: //Don't need to do anything for this yet
			break;

		case KeyAction::CtrlF:
			editor.enableFindMode();
			editor.refreshScreen();

			find(editor);
			break;

		case KeyAction::CtrlS:
			editor.save();
			break;

		case KeyAction::CtrlQ:
			editor.enableExitMode();
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
		case KeyAction::CtrlQ:
		case KeyAction::CtrlS:
		case KeyAction::CtrlF:
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

		do
		{
			std::cout << commandBuffer;
			std::cout.flush();
			editor.updateCommandBuffer(commandBuffer);

			commandInput = getInput();

			if (isActionKey(commandInput)) continue;
			if (commandInput == KeyAction::Esc) return shouldExit;

			if (commandInput == KeyAction::Backspace && command.length() > 0)
			{
				command.pop_back();
			}
			else if (commandInput == KeyAction::Backspace && command.length() == 0) continue;
			else if (commandInput == KeyAction::CtrlBackspace)
			{
				command.clear();
			}
			else if(commandInput != KeyAction::Enter)
			{
				command += static_cast<unsigned char>(commandInput);
			}
			commandBuffer = startStr + command;
		} while (commandInput != KeyAction::Enter);

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

	void find(Editor& editor)
	{
		const char* startStr = "\r\x1b[0KString to find:";
		std::string findString = previousFindString;
		std::string commandBuffer = startStr + findString;
		KeyAction input;

		do
		{
			std::cout << commandBuffer;
			std::cout.flush();
			editor.updateCommandBuffer(commandBuffer);

			input = getInput();

			if (isActionKey(input)) continue;
			if (input == KeyAction::Esc)
			{
				editor.updateCommandBuffer("");
				editor.enableReadMode();
				return;
			}

			if (input == KeyAction::Backspace && findString.length() > 0)
			{
				findString.pop_back();
			}
			else if (input == KeyAction::Backspace && findString.length() == 0) continue;
			else if (input == KeyAction::CtrlBackspace)
			{
				findString.clear();
			}
			else if (input != KeyAction::Enter)
			{
				findString += static_cast<unsigned char>(input);
			}
			commandBuffer = startStr + findString;
		} while (input != KeyAction::Enter);

		previousFindString = findString;
		editor.findString(findString);
	}

	void findModeInput(const KeyAction key, Editor& editor)
	{
		switch (key)
		{
		case KeyAction::ArrowLeft:
		case KeyAction::ArrowRight:
		case KeyAction::ArrowDown:
		case KeyAction::ArrowUp:
		case KeyAction::Enter:
			editor.moveCursorToFind(key);
			break;

		case KeyAction::Esc:
			editor.updateCommandBuffer("");
			editor.enableReadMode();
			break;

		case KeyAction::CtrlS:
			editor.save();
			break;

		case KeyAction::CtrlQ:
			editor.enableExitMode();
			break;

		case KeyAction::CtrlF:
		case static_cast<KeyAction>('f'):
			editor.refreshScreen();

			find(editor);
			break;
		}
	}
}
