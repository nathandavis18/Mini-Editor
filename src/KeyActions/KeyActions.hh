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

/**
* @file KeyActions.hh
* @brief Lists the action keys and their corresponding key code
*/
#pragma once
namespace KeyActions
{
	/// <summary>
	/// The list of action keys and their unique input value
	/// </summary>
	enum class KeyAction
	{
		None = 0,

		CtrlC = 3,
		CtrlF = 6,
		CtrlQ = 17,
		CtrlS = 19,
		CtrlX = 24,
		CtrlY = 25,
		CtrlZ = 26,

		Tab = 9,
		Enter = 13,
		Esc = 27,

#ifdef _WIN32
		Backspace = 8, CtrlBackspace = 127,
#else
		Backspace = 127, CtrlBackspace = 8,
#endif

		ArrowLeft = 1000,	CtrlArrowLeft, //Just give these an arbitrary, unused value to make them unique
		ArrowRight,			CtrlArrowRight,
		ArrowUp,			CtrlArrowUp,
		ArrowDown,			CtrlArrowDown,
		Home,				CtrlHome,
		Delete,				CtrlDelete,
		End,				CtrlEnd,
		PageUp,				CtrlPageUp,
		PageDown,			CtrlPageDown
	};
}