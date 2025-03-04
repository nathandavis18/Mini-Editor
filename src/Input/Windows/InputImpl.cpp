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

#include <conio.h>
#include <cstdint>

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
}