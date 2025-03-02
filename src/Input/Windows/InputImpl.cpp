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

#include "KeyActions/KeyActions.hh"
#include "Input/InputImpl.hpp"
#include "Editor/Editor.hpp"

#include <iostream>
#include <string>
#include <conio.h>

using KeyActions::KeyAction;

namespace InputImpl
{
	const KeyActions::KeyAction getInput()
	{
		uint8_t input = _getch();
		static constexpr uint8_t specialKeyCode = 224;
		static constexpr uint8_t functionKeyCode = 0;
		if (input == functionKeyCode)
		{
			uint8_t _ = _getch(); //Ignore the function key specifier value
			return KeyAction::None; //Don't do anything if a function key (F1, F2, etc.) is pressed
		}
		else if (input == specialKeyCode) //This is the code to determine arrow key presses, delete, end, home, pageup/down
		{
			input = _getch(); //Get the special key identifier
			switch (input)
			{
				//When not holding control
			case 'K': return KeyAction::ArrowLeft;
			case 'M': return KeyAction::ArrowRight;
			case 'P': return KeyAction::ArrowDown;
			case 'H': return KeyAction::ArrowUp;
			case 'S': return KeyAction::Delete;
			case 'O': return KeyAction::End;
			case 'G': return KeyAction::Home;
			case 'Q': return KeyAction::PageDown;
			case 'I': return KeyAction::PageUp;

				//When holding control
			case 's': return KeyAction::CtrlArrowLeft;
			case 't': return KeyAction::CtrlArrowRight;
			case 145: return KeyAction::CtrlArrowDown; //Non-letter ASCII Code
			case 141: return KeyAction::CtrlArrowUp; //Unused ASCII Code
			case 147: return KeyAction::CtrlDelete; //Left Double Quote - Non-standard ASCII Character
			case 'u': return KeyAction::CtrlEnd;
			case 'w': return KeyAction::CtrlHome;
			case 'v': return KeyAction::CtrlPageDown;
			case 134: return KeyAction::CtrlPageUp; //Non-letter ASCII Code
			}
		}
		return static_cast<KeyAction>(input);
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

	bool doCommand()
	{
		bool shouldExit = false;
		const char* startStr = "\x1b[2K\x1b[1A\x1b[1E:";
		std::string command;
		std::string commandBuffer = startStr;
		KeyAction commandInput;
		std::cout << commandBuffer;
		Editor::updateCommandBuffer(commandBuffer);
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
			Editor::updateCommandBuffer(commandBuffer);
			std::cout << commandBuffer;
		}

		if ((command == "q" && !Editor::isDirty()) || command == "q!") //Quit command - requires changes to be saved if not force quit
		{
			Editor::enableExitMode();
			shouldExit = true;
		}
		else if (command == "w" || command == "s") //Save commands ([w]rite / [s]ave)
		{
			Editor::save();
		}
		else if (command == "wq" || command == "sq") //Save and quit commands ([w]rite [q]uit / [s]ave [q]uit)
		{
			Editor::save();
			Editor::enableExitMode();
			shouldExit = true;
		}
		return shouldExit;
	}
}