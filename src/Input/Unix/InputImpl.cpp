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
#include "Input/GetInputImpl.hpp"
#include "Editor/Editor.hpp"

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <iostream>

using KeyActions::KeyAction;

namespace InputImpl
{
	/// <summary>
	/// A custom implementation of the _getch function from Windows
	/// </summary>
	/// <returns></returns>
	const KeyAction getInput()
	{
		int8_t nread;
		char c;
		while (nread = read(fileno(stdin), &c, 1) == 0); //While nothing is being read
		if (nread == -1) exit(EXIT_FAILURE); //You've met with a terrible fate, haven't you

		if (c == static_cast<char>(KeyAction::Esc))
		{
			char seq[3];
			if (read(fileno(stdin), seq, 3) < 2) return KeyAction::Esc;
			if (seq[0] == '[')
			{
				if (seq[2] == static_cast<char>(KeyAction::None)) //If 3 characters weren't read in
				{
					switch (seq[1])
					{
					case 'A': return KeyAction::ArrowUp;
					case 'B': return KeyAction::ArrowDown;
					case 'C': return KeyAction::ArrowRight;
					case 'D': return KeyAction::ArrowLeft;
					case 'H': return KeyAction::Home;
					case 'F': return KeyAction::End;
					}
				}
				else
				{
					if (seq[2] == '~')
					{
						switch (seq[1])
						{
						case '3': return KeyAction::Delete;
						case '5': return KeyAction::PageUp;
						case '6': return KeyAction::PageDown;
						}
					}
					else if (seq[2] == ';')
					{
						switch (seq[1])
						{
						case '1':
							if (read(fileno(stdin), seq, 2) < 2) return KeyAction::Esc;
							if (seq[0] == '5')
							{
								switch (seq[1])
								{
								case 'A': return KeyAction::CtrlArrowUp;
								case 'B': return KeyAction::CtrlArrowDown;
								case 'C': return KeyAction::CtrlArrowRight;
								case 'D': return KeyAction::CtrlArrowLeft;
								case 'H': return KeyAction::CtrlHome;
								case 'F': return KeyAction::CtrlEnd;
								}
							}
						case '3': return KeyAction::CtrlDelete;
						case '5':
							if (read(fileno(stdin), seq, 2) < 2) return KeyAction::Esc;
							return KeyAction::CtrlPageUp;
						case '6':
							if (read(fileno(stdin), seq, 2) < 2) return KeyAction::Esc;
							return KeyAction::CtrlPageDown;
						}
					}
				}
			}
			else if (seq[0] == 'O')
			{
				switch (seq[1])
				{
				case 'H': return KeyAction::Home;
				case 'F': return KeyAction::End;
				}
			}
		}
		return static_cast<KeyAction>(c);
	}

	bool doCommand()
	{
		bool shouldExit = false;
		std::string command;
		std::cout << ":";
		std::cin >> command;

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