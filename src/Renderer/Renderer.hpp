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
#include <cstdint>
#include <string>
#include <string_view>
class Renderer
{
public:
	/// <summary>
	/// Initializes the text buffer with a specific ANSI Escape Code
	/// </summary>
	Renderer();

	/// <summary>
	/// Adds the rendered line to the text buffer to later be rendered
	/// </summary>
	/// <param name="renderedLine"></param>
	void addRenderedLineToBuffer(const std::string& renderedLine);

	/// <summary>
	/// If we have reached EOF and there is still more to render, add EOF markers to the buffer
	/// </summary>
	/// <param name="rowsToEnter"></param>
	/// <param name="colCount"></param>
	/// <param name="emptyFile"></param>
	void addEndOfFileToBuffer(const uint16_t rowsToEnter, const uint16_t colCount, const bool emptyFile);

	/// <summary>
	/// Renders the main text buffer, status buffer, cursor buffer, and command buffer when applicable
	/// </summary>
	/// <param name="forceDraw"></param>
	/// <param name="renderCommandBuffer"></param>
	void renderScreen(const bool forceDraw, const bool renderCommandBuffer);

	/// <summary>
	/// Sets the status buffer with all the necessary status information
	/// </summary>
	/// <param name="statusRowStart"></param>
	/// <param name="dirty"></param>
	/// <param name="fileName"></param>
	/// <param name="numRows"></param>
	/// <param name="currentRow"></param>
	/// <param name="currentCol"></param>
	/// <param name="mode"></param>
	/// <param name="rStatus"></param>
	/// <param name="maxLength"></param>
	void setStatusBuffer(const uint16_t statusRowStart, const bool dirty, const std::string_view fileName, const size_t numRows, const size_t currentRow, const size_t currentCol, const std::string& mode, const std::string& rStatus, const size_t maxLength);

	/// <summary>
	/// Sets the cursor buffer responsible for displaying the cursor in the correct position
	/// </summary>
	/// <param name="cursorRow"></param>
	/// <param name="cursorCol"></param>
	void setCursorBuffer(const uint16_t cursorRow, const uint16_t cursorCol);

	/// <summary>
	/// Updates the command buffer with its location and the string to display
	/// </summary>
	/// <param name="commandBuffer"></param>
	/// <param name="commandBufferRow"></param>
	void setCommandBuffer(const std::string& commandBuffer, const size_t commandBufferRow);

	/// <summary>
	/// Clears the terminal
	/// </summary>
	static void clearScreen();

private:
	std::string mTextRenderBuffer, mPreviousTextRenderBuffer;
	std::string mCursorBuffer, mStatusBuffer;
	std::string mCommandBuffer;
};