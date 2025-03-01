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

#pragma once

class Console
{
public:
	/// <summary>
	/// Used to maintain the size of the console window
	/// </summary>
	struct WindowSize
	{
		int rows{}, cols{};
	};

	/// <summary>
	/// Initializes the console by setting the window size, getting default mode parameters, and 
	/// </summary>
	Console();

	/// <summary>
	/// Should be called when outside files/classes need access to the console size (i.e. the editor)
	/// </summary>
	/// <returns> An object containing row and col sizes </returns>
	WindowSize getWindowSize();

	/// <summary>
	/// Enables raw input mode on the console by using the OS-specific functions
	/// </summary>
	/// <returns> Returns true if successful </returns>
	void enableRawInput();

	/// <summary>
	/// Disables raw input mode by using the OS-specific API to return to default mode.
	/// </summary>
	void disableRawInput();

private:
	/// <summary>
		/// Gets the default console mode from the OS and sets a local OS-Specific variable with the console settings
		/// </summary>
	void setDefaultMode();

	/// <summary>
	/// Uses the OS API to get console size information and stores it in the local WindowSize object
	/// </summary>
	void setWindowSize();

	/// <summary>
	/// If the program exits due to an error, force it back into default mode
	/// </summary>
	static void forceDisableRawInput();

private:
	bool rawModeEnabled = false;
	WindowSize mWindowSize;
};