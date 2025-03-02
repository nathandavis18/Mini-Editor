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

#include "Input.hpp"
#include "Editor/Editor.hpp"
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

	void changeMode(const KeyAction key)
	{
		bool shouldExit = false;
		switch (key)
		{
		case static_cast<KeyAction>('i'):
			Editor::enableEditMode();
			break;
		case static_cast<KeyAction>(':'):
			Editor::enableCommandMode();
			Editor::refreshScreen();

			shouldExit = InputImpl::doCommand();
			Editor::updateCommandBuffer(std::string_view());

			if (!shouldExit)
			{
				Editor::enableReadMode(); //Go back to read mode after executing a command
				Editor::refreshScreen();
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
			Editor::moveCursor(key);
			break;

		case KeyAction::CtrlArrowDown:
		case KeyAction::CtrlArrowUp:
		case KeyAction::PageDown:
		case KeyAction::PageUp:
			Editor::shiftRowOffset(key);
			break;

		default: //Unknown command. Just go back to read mode
			Editor::enableReadMode();
			break;
		}
	}

	void handleInput(KeyAction key)
	{
		switch (key)
		{
		case KeyAction::Esc:
			Editor::enableReadMode();
			break;
		case KeyAction::Delete:
		case KeyAction::Backspace:
		case KeyAction::CtrlBackspace:
		case KeyAction::CtrlDelete:
			Editor::deleteChar(key);
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
			Editor::moveCursor(key);
			break;
		case KeyAction::CtrlArrowDown:
		case KeyAction::CtrlArrowUp:
			Editor::shiftRowOffset(key);
			break;
		case KeyAction::Enter:
			Editor::addRow();
			break;
		case KeyAction::CtrlZ:
			Editor::undoChange();
			break;
		case KeyAction::CtrlY:
			Editor::redoChange();
			break;
		case KeyAction::CtrlC: //Don't need to do anything for this yet
			break;

		default: [[ likely ]] //This is the most likely scenario
			Editor::insertChar(static_cast<uint8_t>(key));
			break;
		}
	}
}
