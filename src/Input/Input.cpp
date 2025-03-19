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
	std::string replaceString = "";

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
			shouldExit = doCommand(editor);

			if (!shouldExit)
			{
				editor.enableReadMode(); //Go back to read mode after executing a command
			}
			break;

		case KeyAction::CtrlF:
		case static_cast<KeyAction>('f'):
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
		case KeyAction::CtrlPageDown:
		case KeyAction::CtrlPageUp:
			editor.moveCursor(key);
			break;

		case KeyAction::PageDown:
		case KeyAction::PageUp:
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
			editor.enableFindInputMode();
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

	std::string getCommandInput(Editor& editor, const std::string& startStr, bool findMode = false)
	{
		std::string inputStr;
		if(findMode) inputStr = previousFindString;
		std::string commandBuffer = startStr + inputStr;

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
				return std::string();
			}

			if (input == KeyAction::Backspace && inputStr.length() > 0)
			{
				inputStr.pop_back();
			}
			else if (input == KeyAction::Backspace && inputStr.empty()) continue;
			else if (input == KeyAction::CtrlBackspace)
			{
				inputStr.clear();
			}
			else if (input != KeyAction::Enter)
			{
				inputStr += static_cast<unsigned char>(input);
			}
			commandBuffer = startStr + inputStr;
		} while (input != KeyAction::Enter);

		if(findMode) previousFindString = inputStr;

		return inputStr;
	}

	bool doCommand(Editor& editor)
	{
		editor.enableCommandMode();
		editor.refreshScreen();
		bool shouldExit = false;
		const char* startStr = "\r\x1b[0K:";
		std::string command = getCommandInput(editor, startStr);

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

		editor.updateCommandBuffer(std::string());
		return shouldExit;
	}

	void find(Editor& editor)
	{
		editor.enableFindInputMode();
		editor.refreshScreen();

		const char* startStr = "\r\x1b[0KString to find:";
		std::string findString = getCommandInput(editor, startStr, true);
		if (!findString.empty())
		{
			editor.enableFindMode();
			editor.findString(findString);
		}
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
			find(editor);
			break;

		case static_cast<KeyAction>('r'):
			replace(editor);
			break;
		}
	}

	void replace(Editor& editor)
	{
		editor.enableReplaceInputMode();
		editor.refreshScreen();

		const char* startStr = "\r\x1b[0KReplace With:";
		std::string replaceStr = getCommandInput(editor, startStr);
		replaceString = replaceStr;
		editor.enableReplaceMode();
	}

	void replaceModeInput(const KeyAction key, Editor& editor)
	{
		switch (key)
		{
		case KeyAction::Enter:
			editor.replaceFindString(replaceString);
			break;

		case static_cast<KeyAction>('a'):
			editor.replaceFindString(replaceString, true);
			break;

		case KeyAction::ArrowLeft:
		case KeyAction::ArrowRight:
		case KeyAction::ArrowDown:
		case KeyAction::ArrowUp:
			editor.moveCursorToFind(key);
			break;

		case KeyAction::Esc:
			editor.updateCommandBuffer("");
			replaceString.clear();
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
			find(editor);
			break;

		case static_cast<KeyAction>('r'):
			replace(editor);
			break;
		}
	}
}
