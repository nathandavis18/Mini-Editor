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

#pragma once
#include "Editor/Editor.hpp"
#include "KeyActions/KeyActions.hh"

namespace InputHandler
{
	/// <summary>
	/// Calls the implementation-specific getInput.
	/// Called on every input
	/// </summary>
	/// <returns></returns>
	const KeyActions::KeyAction getInput();

	/// <summary>
	/// Handles inputs while in command/read mode
	/// Current options include:
	///		i = Enter edit mode (like VIM)
	///		: = Enter command mode (like VIM)
	/// </summary>
	void changeMode(const KeyActions::KeyAction, Editor& editor);

	/// <summary>
	/// Handles the input while in EDIT mode. As such, is only called while in EDIT mode
	/// </summary>
	/// <param name=""></param>
	void handleInput(const KeyActions::KeyAction, Editor& editor);

	/// <summary>
	/// Handles the command input when in command mode.
	/// Functionality changed to allow for non busy-looping implementation
	/// </summary>
	/// <returns> True if program should exit, false otherwise </returns>
	bool doCommand(Editor& editor);

	/// <summary>
	/// Gets the find string from the user in a similar fashion to inputting a command
	/// </summary>
	/// <param name="editor"></param>
	void find(Editor& editor);

	/// <summary>
	/// Controls what happens when find mode is enabled and the user presses a key
	/// </summary>
	/// <param name=""></param>
	/// <param name="editor"></param>
	void findModeInput(const KeyActions::KeyAction, Editor& editor);

	void replace(Editor& editor);

	void replaceModeInput(const KeyActions::KeyAction, Editor& editor);
}