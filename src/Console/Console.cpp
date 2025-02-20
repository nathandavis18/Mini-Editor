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

/**
* @file Console.cpp
* @brief Provides the implementation for handling terminal output and cursor movement
* 
* Controls everything that is displayed to the console, as well as its position relative to the file. This includes:
* Cursor position, rendered text, as well as how the cursor moves through the file. Also includes file history for undo/redo
* Essentially, this is the 'editor' of the Text Editor. The core piece that binds this project together.
*/

#include "Console.hpp"

#include <iostream>
#include <fstream>
#include <format> //C++20 is required. MSVC/GCC-13/Clang-14/17/AppleClang-15

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#endif
#define NotVimVersion "0.5.1a"

/// <summary>
/// Default Construct the window
/// </summary>
Console::Window::Window() : fileCursorX(0), fileCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0), colNumberToDisplay(0), savedRenderedCursorXPos(0),
rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), fileRows(FileHandler::loadFileContents())
{}

/// <summary>
/// Sets/Gets the current mode the editor is in
/// </summary>
/// <param name="m"> The new mode </param>
/// <returns> The current mode </returns>
Mode& Console::mode(Mode m)
{
	if (m != Mode::None)
	{
		mMode = m;
	}
	return mMode;
}

/// <summary>
/// Steps to be taken before refreshScreen() is called
/// </summary>
void Console::prepRenderedString()
{
	if (mWindow->fileRows.size() > 0 && mMode != Mode::CommandMode) fixRenderedCursorPosition(mWindow->fileRows.at(mWindow->fileCursorY));
	setRenderedString();
	setHighlight();
}

/// <summary>
/// Preps the rendered string by replacing tabs with necessary spaces
/// </summary>
void Console::setRenderedString()
{
	for (size_t r = 0; r < mWindow->fileRows.size(); ++r)
	{
		if (r > mWindow->rowOffset + mWindow->rows) return;

		FileHandler::Row& row = mWindow->fileRows.at(r);
		row.renderedLine = row.line;
		if (row.renderedLine.length() > 0 && (r >= mWindow->rowOffset && r <= mWindow->rowOffset + mWindow->rows))
		{
			replaceRenderedStringTabs(row.renderedLine);
		}
	}
}

/// <summary>
/// Renders the status bar and cursor separately from the main text
/// </summary>
void Console::renderStatusAndCursor() 
{
	const uint8_t statusRowStart = mWindow->rows + 1;
	constexpr uint8_t statusColStart = 0;
	std::string statusAndCursorBuffer = std::format("\x1b[{};{}H", statusRowStart, statusColStart); //Move the cursor to this position to draw the status bar

	statusAndCursorBuffer.append("\x1b[7m"); //Set to inverse color mode (white background dark text) for status row

	std::string rStatus, modeToDisplay;
	std::string status = std::format("{} - {} lines {}", FileHandler::fileName(), mWindow->fileRows.size(), mWindow->dirty ? "(modified)" : "");

	//Set the displayed mode and the cursor position for display
	size_t currentRowToDisplay = mWindow->rowOffset + mWindow->renderedCursorY + 1; //Add 1 for display only. Rows and cols are 0-indexed internally
	size_t currentColToDisplay = mWindow->colNumberToDisplay + 1;
	if (mMode == Mode::EditMode)
	{
		rStatus = std::format("row {}/{} col {}", currentRowToDisplay, mWindow->fileRows.size(), currentColToDisplay);
		modeToDisplay = "EDIT";
	}
	else if (mMode == Mode::CommandMode)
	{
		rStatus = "Enter command";
		modeToDisplay = "COMMAND";
	}
	else if (mMode == Mode::ReadMode)
	{
		rStatus = std::format("row {}/{} col {}", currentRowToDisplay, mWindow->fileRows.size(), currentColToDisplay);
		modeToDisplay = "READ ONLY";
	}
	
	size_t statusLength = (status.length() > mWindow->cols) ? mWindow->cols : status.length();
	if (statusLength < status.length())
	{
		status.resize(statusLength);
	}

	statusAndCursorBuffer.append(status);

	while (statusLength < (mWindow->cols / 2))
	{
		if ((mWindow->cols / 2) - statusLength == modeToDisplay.length() / 2)
		{
			statusAndCursorBuffer.append(modeToDisplay);
			break;
		}
		else
		{
			statusAndCursorBuffer.append(" ");
			++statusLength;
		}
	}
	statusLength += modeToDisplay.length();

	while (statusLength < mWindow->cols)
	{
		if (mWindow->cols - statusLength == rStatus.length())
		{
			statusAndCursorBuffer.append(rStatus);
			break;
		}
		else
		{
			statusAndCursorBuffer.append(" ");
			++statusLength;
		}
	}

	statusAndCursorBuffer.append("\x1b[0m\r\n"); //Set to default mode

	size_t rowToDisplay = mWindow->renderedCursorY + 1; //Add 1 for display only. Actual rows/cols are 0-indexed internally
	size_t colToDisplay = mWindow->renderedCursorX + 1;
	std::string cursorPosition = std::format("\x1b[{};{}H", rowToDisplay, colToDisplay); //Move the cursor to this position after rendering status bar
	statusAndCursorBuffer.append(cursorPosition);

	std::cout << statusAndCursorBuffer; //Send the status bar and cursor to be rendered
}

/// <summary>
/// Makes sure the rendered line has only contains text that should be on screen
/// </summary>
void Console::prepRenderedLineForRender() 
{
	for (size_t y = mWindow->rowOffset; y < mWindow->fileRows.size() && y < mWindow->rows + mWindow->rowOffset; ++y)
	{
		FileHandler::Row& row = mWindow->fileRows.at(y);

		//Set the render string length to the lesser of the terminal width and the line length.
		const size_t renderedLength = (row.renderedLine.length() - mWindow->colOffset) > mWindow->cols ? mWindow->cols : row.renderedLine.length();
		if (renderedLength > 0)
		{
			if (mWindow->colOffset < row.renderedLine.length())
			{
				row.renderedLine = row.renderedLine.substr(mWindow->colOffset, renderedLength);
			}
			else
			{
				row.renderedLine.clear();
			}
		}
		else
		{
			row.renderedLine.clear();
		}
	}
}

/// <summary>
/// What gets rendered if EOF is reached before end of display is reached
/// </summary>
void Console::renderEndOfFile() 
{
	const char* emptyRowCharacter = "~";
	for (size_t y = mWindow->rowOffset; y < mWindow->rows + mWindow->rowOffset; ++y)
	{
		if (y >= mWindow->fileRows.size())
		{
			if (mWindow->fileRows.size() == 0 && y == mWindow->rows / 3) //If the file is empty and the current row is at 1/3 height (good display position)
			{
				std::string welcome = std::format("NotVim Editor -- version {}\x1b[0K\r\n", NotVimVersion);
				size_t padding = (mWindow->cols - welcome.length()) / 2;
				if (padding > 0)
				{
					mRenderBuffer.append(emptyRowCharacter);
					--padding;
				}
				while (padding > 0)
				{
					mRenderBuffer.append(" ");
					--padding;
				}
				mRenderBuffer.append(welcome);
			}
			else
			{
				mRenderBuffer.append("\x1b[0m"); //Make sure color mode is back to normal
				mRenderBuffer.append(emptyRowCharacter);
				mRenderBuffer.append("\x1b[0K\r\n"); //Clear the rest of the row
			}
			continue;
		}
	}
}

/// <summary>
/// Builds the output buffer and displays it to the user through std::cout
/// Uses ANSI escape codes for clearing screen/displaying cursor and for colors
/// Currently redraws entire screen if buffer is different. Will work on making it only redraw the changes that were made
/// The issue right now is multiline syntax highlighting.
/// </summary>
void Console::refreshScreen(bool forceRedrawScreen)
{
	mRenderBuffer = "\x1b[1;1H"; //Move cursor to start of screen to redraw changes.

	prepRenderedLineForRender();
	updateRenderedColor(mWindow->rowOffset, mWindow->colOffset);
	for (size_t i = mWindow->rowOffset; i < mWindow->fileRows.size() && i < mWindow->rowOffset + mWindow->rows; ++i)
	{
		mRenderBuffer.append(mWindow->fileRows.at(i).renderedLine);
		mRenderBuffer.append("\x1b[0K\r\n");
	}

	mRenderBuffer.append("\x1b[0m"); //Make sure color mode is back to normal

	if (mWindow->rowOffset + mWindow->rows >= mWindow->fileRows.size())
	{
		renderEndOfFile();
	}
	
	mRenderBuffer.append("\x1b[3J"); //Remove saved lines (so there is no scroll bar)

	if (mRenderBuffer != mPreviousRenderBuffer || forceRedrawScreen) 
	{
		std::cout << mRenderBuffer;
		mPreviousRenderBuffer = mRenderBuffer;
	}

	renderStatusAndCursor();

	std::cout.flush(); //Flush the buffer after rendering everything to screen
}

/// <summary>
/// Arrow Left/Right and Ctrl-Arrow Left/Right have the same functionality in a few scenarios
/// If one of those scenarios is the case, act accordingly
/// </summary>
/// <param name="key"></param>
/// <returns></returns>
int8_t Console::moveCursorLeftRight(const KeyActions::KeyAction key)
{
	bool isForward = (key == KeyActions::KeyAction::ArrowRight || key == KeyActions::KeyAction::CtrlArrowRight);

	if (isForward)
	{
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1 
		 && mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length()) return cursorCantMove; //Can't move any farther right if we are at the end of the file

		if (mWindow->fileCursorX == mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			++mWindow->fileCursorY;
			mWindow->fileCursorX = 0;
			return cursorMovedNewLine;
		}

		if (key == KeyActions::KeyAction::ArrowRight) ++mWindow->fileCursorX;
	}
	else
	{
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return cursorCantMove;

		if (mWindow->fileCursorX == 0)
		{
			--mWindow->fileCursorY;
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
			return cursorMovedNewLine;
		}

		if (key == KeyActions::KeyAction::ArrowLeft) --mWindow->fileCursorX;
	}
	return cursorMoveNormal;
}

/// <summary>
/// Controls the different cursor movement operations
/// Called when a movement key is pressed, including:
///		Ctrl Page Up/Down
///		Arrow Up/Left/Right/Down	Ctrl Arrow Left/Right
///		Home/End					Ctrl Home/End
/// </summary>
/// <param name="key">The arrow key pressed</param>
void Console::moveCursor(const KeyActions::KeyAction key)
{
	int8_t returnCode = 0;
	if (key == KeyActions::KeyAction::ArrowLeft || key == KeyActions::KeyAction::ArrowRight 
		|| key == KeyActions::KeyAction::CtrlArrowLeft || key == KeyActions::KeyAction::CtrlArrowRight)
	{
		returnCode = moveCursorLeftRight(key);
		if (returnCode == cursorCantMove) return;
	}

	if (key != KeyActions::KeyAction::ArrowUp && key != KeyActions::KeyAction::ArrowDown)
	{
		mWindow->updateSavedPos = true;
	}

	switch (key)
	{
	case KeyActions::KeyAction::ArrowUp:
		if (mWindow->fileCursorY == 0)
		{
			mWindow->fileCursorX = 0;
			return;
		}

		--mWindow->fileCursorY;
		setCursorLinePosition();
		break;

	case KeyActions::KeyAction::ArrowDown:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
			return;
		}

		++mWindow->fileCursorY;
		setCursorLinePosition();
		break;

	case KeyActions::KeyAction::CtrlArrowLeft:
		if(returnCode == cursorMoveNormal)
		{
			while (mWindow->fileCursorX > 0)
			{
				--mWindow->fileCursorX;
				char charToFind = mWindow->fileRows.at(mWindow->fileCursorY).line[mWindow->fileCursorX];
				if (separators.find(charToFind) != std::string::npos) break;
			}
		} 
		break;

	case KeyActions::KeyAction::CtrlArrowRight:
		if(returnCode == cursorMoveNormal)
		{
			while (mWindow->fileCursorX < mWindow->fileRows.at(mWindow->fileCursorY).line.length())
			{
				++mWindow->fileCursorX;
				char charToFind = mWindow->fileRows.at(mWindow->fileCursorY).line[mWindow->fileCursorX];
				if (separators.find(charToFind) != std::string::npos) break;
			}
		}

		break;

	case KeyActions::KeyAction::Home:
		mWindow->fileCursorX = 0;
		break;
		
	case KeyActions::KeyAction::End:
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		break;

	case KeyActions::KeyAction::CtrlHome:
		mWindow->fileCursorX = 0; mWindow->fileCursorY = 0;
		break;

	case KeyActions::KeyAction::CtrlEnd:
		mWindow->fileCursorY = mWindow->fileRows.size() - 1;
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		break;

	case KeyActions::KeyAction::CtrlPageUp: //Move cursor to top of screen
		mWindow->fileCursorY -= (mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows;
		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;

	case KeyActions::KeyAction::CtrlPageDown: //Move cursor to bottom of screen
		if (mWindow->fileCursorY + mWindow->rows - ((mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows) > mWindow->fileRows.size() - 1)
		{
			mWindow->fileCursorY = mWindow->fileRows.size() - 1;
		}
		else
		{
			mWindow->fileCursorY += mWindow->rows - ((mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows);
		}

		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;

	default:
		break;
	}
}


/// <summary>
/// When a key that shifts the row offset is pressed, handle it accordingly
/// Keys include CTRL Arrow-Up/Down, Page Up/Down
/// </summary>
/// <param name="key">The key pressed</param>
void Console::shiftRowOffset(const KeyActions::KeyAction key)
{
	switch (key)
	{
	case KeyActions::KeyAction::CtrlArrowDown:
		if (mWindow->rowOffset == mWindow->fileRows.size() - 1) return; //This is as far as the screen can be moved down

		++mWindow->rowOffset;
		if (mWindow->fileCursorY < mWindow->fileRows.size() && mWindow->renderedCursorY == 0) //Move the file cursor if the rendered cursor is at the top of the screen
		{
			moveCursor(KeyActions::KeyAction::ArrowDown);
		}
		break;

	case KeyActions::KeyAction::CtrlArrowUp:
		if (mWindow->rowOffset == 0) return; //A negative row offset would wrap and break the viewport so don't allow it to go negative

		--mWindow->rowOffset;
		if (mWindow->renderedCursorY == mWindow->rows - 1) //Move the file cursor if the rendered cursor is at the bottom of the screen
		{
			moveCursor(KeyActions::KeyAction::ArrowUp);
		}
		break;

	case KeyActions::KeyAction::PageUp: //Shift screen offset up by 1 page worth (mWindow->rows)
		if (mWindow->fileCursorY < mWindow->rows)
		{
			mWindow->fileCursorY = 0;
			mWindow->rowOffset = 0;
		}
		else
		{
			mWindow->fileCursorY -= mWindow->rows;
			if (mWindow->rowOffset >= mWindow->rows) mWindow->rowOffset -= mWindow->rows;
			else mWindow->rowOffset = 0;
		}
		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;

	case KeyActions::KeyAction::PageDown: //Shift screen offset down by 1 page worth (mWindow->rows)
		if (mWindow->fileCursorY + mWindow->rows > mWindow->fileRows.size() - 1)
		{
			if (mWindow->fileCursorY == mWindow->fileRows.size() - 1) return;

			mWindow->fileCursorY = mWindow->fileRows.size() - 1;
			mWindow->rowOffset += mWindow->fileCursorY % mWindow->rows;
		}
		else
		{
			mWindow->fileCursorY += mWindow->rows;
			mWindow->rowOffset += mWindow->rows;
		}
		if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		}
		break;

	default:
		break;
	}
}

/// <summary>
/// Adds a new row when ENTER/RETURN is pressed
/// Moves data past the cursor onto the new row
/// </summary>
void Console::addRow()
{
	addUndoHistory();

	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);

	if (mWindow->fileCursorX == row.line.length())
	{
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY + 1, FileHandler::Row());
	}
	else if (mWindow->fileCursorX == 0)
	{
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY, FileHandler::Row());
	}
	else
	{
		FileHandler::Row newRow;
		newRow.line = row.line.substr(mWindow->fileCursorX);
		row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.end());
		mWindow->fileRows.insert(mWindow->fileRows.begin() + mWindow->fileCursorY + 1, newRow);
	}

	mWindow->fileCursorX = 0; ++mWindow->fileCursorY;
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

/// <summary>
/// Deletes a row when the last character in the row is removed
/// </summary>
/// <param name="rowNum">The row to be deleted</param>
void Console::deleteRow(const size_t rowNum)
{
	if (rowNum > mWindow->fileRows.size()) return;
	mWindow->fileRows.erase(mWindow->fileRows.begin() + rowNum);
	mWindow->dirty = true;
}

/// <summary>
/// Deletes a character behind/ahead of the cursor depending on key pressed
/// </summary>
/// <param name="key">Delete or Backspace, or their CTRL variants</param>
void Console::deleteChar(const KeyActions::KeyAction key)
{
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);
	addUndoHistory();
	switch (key)
	{
	case KeyActions::KeyAction::Backspace:
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0)
		{
			mUndoHistory.pop();
			return;
		}

		if (mWindow->fileCursorX == 0)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY - 1).line.length();
			mWindow->fileRows.at(mWindow->fileCursorY - 1).line.append(row.line);
			deleteRow(mWindow->fileCursorY);
			--mWindow->fileCursorY;
		}
		else
		{
			row.line.erase(row.line.begin() + mWindow->fileCursorX - 1);
			--mWindow->fileCursorX;
		}
		break;

	case KeyActions::KeyAction::Delete:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1 && mWindow->fileCursorX == row.line.length())
		{
			mUndoHistory.pop();
			return;
		}

		if (mWindow->fileCursorX == row.line.length())
		{
			row.line.append(mWindow->fileRows.at(mWindow->fileCursorY + 1).line);
			deleteRow(mWindow->fileCursorY + 1);
		}
		else
		{
			row.line.erase(row.line.begin() + mWindow->fileCursorX);
		}
		break;

	case KeyActions::KeyAction::CtrlBackspace:
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0)
		{
			mUndoHistory.pop();
			return;
		}

		if (mWindow->fileCursorX == 0)
		{
			mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY - 1).line.length();
			mWindow->fileRows.at(mWindow->fileCursorY - 1).line.append(row.line);
			deleteRow(mWindow->fileCursorY);
			--mWindow->fileCursorY; 
		}
		else
		{
			size_t findPos;
			if ((findPos = row.line.substr(0, mWindow->fileCursorX).find_last_of(separators)) == std::string::npos) //Delete everything in the row to the beginning
			{
				row.line.erase(row.line.begin(), row.line.begin() + mWindow->fileCursorX);
				mWindow->fileCursorX = 0;
			}
			else if (findPos == mWindow->fileCursorX - 1)
			{
				deleteChar(KeyActions::KeyAction::Backspace); //Delete just the separator
			}
			else
			{
				row.line.erase(row.line.begin() + findPos + 1, row.line.begin() + mWindow->fileCursorX);
				mWindow->fileCursorX = findPos + 1;
			}
		}
		break;

	case KeyActions::KeyAction::CtrlDelete:
		if (mWindow->fileCursorY == mWindow->fileRows.size() - 1 && mWindow->fileCursorX == row.line.length())
		{
			mUndoHistory.pop();
			return;
		}

		if (mWindow->fileCursorX == row.line.length())
		{
			row.line.append(mWindow->fileRows.at(mWindow->fileCursorY + 1).line);
			deleteRow(mWindow->fileCursorY + 1);
		}
		else
		{
			size_t findPos;
			if ((findPos = row.line.substr(mWindow->fileCursorX).find_first_of(separators)) == std::string::npos) //Delete everything in the row to the beginning
			{
				row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.end());
			}
			else if (findPos == 0)
			{
				deleteChar(KeyActions::KeyAction::Delete); //Delete just the separator
			}
			else
			{
				row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.begin() + findPos + mWindow->fileCursorX);
			}
		}
		break;
	}
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

/// <summary>
/// Inserts a given character at the current position and moves the cursor forward
/// This is what will typically be called on keyboard input
/// </summary>
/// <param name="c">The character to insert</param>
void Console::insertChar(const unsigned char c)
{
	FileHandler::Row& row = mWindow->fileRows.at(mWindow->fileCursorY);

	addUndoHistory();

	row.line.insert(row.line.begin() + mWindow->fileCursorX, c);
	++mWindow->fileCursorX;
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

/// <summary>
/// Adds the last change made to the undo history
/// Currently every change made gets added
/// I am planning to have it on a timer so if a bunch of changes happen at once they will all be on the same history stack
/// </summary>
void Console::addUndoHistory()
{
	FileHistory history;
	history.rows = mWindow->fileRows;
	history.fileCursorX = mWindow->fileCursorX;
	history.fileCursorY = mWindow->fileCursorY;
	history.colOffset = mWindow->colOffset;
	history.rowOffset = mWindow->rowOffset;

	mUndoHistory.push(history);
}

/// <summary>
/// Adds the last undo change to the redo history
/// </summary>
void Console::addRedoHistory()
{
	FileHistory history;
	history.rows = mWindow->fileRows;
	history.fileCursorX = mWindow->fileCursorX;
	history.fileCursorY = mWindow->fileCursorY;
	history.colOffset = mWindow->colOffset;
	history.rowOffset = mWindow->rowOffset;

	mRedoHistory.push(history);
}

/// <summary>
/// Add the current changes to the redo stack to be able to be redone, then undo the last change.
/// </summary>
void Console::undoChange()
{
	if (mUndoHistory.size() == 0) return;

	addRedoHistory();

	mWindow->fileRows = mUndoHistory.top().rows;
	mWindow->fileCursorX = mUndoHistory.top().fileCursorX;
	mWindow->fileCursorY = mUndoHistory.top().fileCursorY;
	mWindow->colOffset = mUndoHistory.top().colOffset;
	mWindow->rowOffset = mUndoHistory.top().rowOffset;

	mUndoHistory.pop();
}

/// <summary>
/// Redo the last change that CTRL-Z undid and pop the redo change from the redoHistory stack.
/// </summary>
void Console::redoChange()
{
	if (mRedoHistory.size() == 0) return;

	addUndoHistory();

	mWindow->fileRows = mRedoHistory.top().rows;
	mWindow->fileCursorX = mRedoHistory.top().fileCursorX;
	mWindow->fileCursorY = mRedoHistory.top().fileCursorY;
	mWindow->colOffset = mRedoHistory.top().colOffset;
	mWindow->rowOffset = mRedoHistory.top().rowOffset;

	mRedoHistory.pop();
}

bool Console::isRawMode()
{
	return mWindow->rawModeEnabled;
}

bool Console::isDirty()
{
	return mWindow->dirty;
}

/// <summary>
/// Saves the file and sets dirty = false
/// Appends a newline character to the end of each row, assuming it's not the last row so that when written to the file the contents are saved properly
/// </summary>
void Console::save()
{
	std::string output;
	for (size_t i = 0; i < mWindow->fileRows.size(); ++i)
	{
		if (i == mWindow->fileRows.size() - 1)
		{
			output.append(mWindow->fileRows.at(i).line);
		}
		else [[ likely ]]
		{
			output.append(mWindow->fileRows.at(i).line + "\r\n");
		}
	}
	FileHandler::saveFile(output);
	mWindow->dirty = false;
}

/// <summary>
/// Moves the rendered cursor to the command mode area and enable command mode
/// </summary>
void Console::enableCommandMode()
{
	disableRawInput();
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + statusMessageRows;
	mMode = Mode::CommandMode;

	prepRenderedString();
	refreshScreen();
}

/// <summary>
/// Enables edit mode. If the file is empty, add an empty row to start the file
/// </summary>
void Console::enableEditMode()
{
	if (mWindow->fileRows.size() == 0)
	{
		mWindow->fileRows.push_back(FileHandler::Row());
	}
	mMode = Mode::EditMode;
}

/// <summary>
/// Allows for smooth movement of the cursor when moving up/down
/// Compares the last value since the cursor was moved left/right (either by inserting/deleting character or moving left/right manually)
/// Finds the closest position to the last saved value without going past it
/// </summary>
void Console::setCursorLinePosition()
{
	if (mWindow->renderedCursorX > mWindow->fileRows.at(mWindow->fileCursorY).renderedLine.length())
	{
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
		return;
	}
	mWindow->fileCursorX = 0;
	size_t spaces = getRenderedCursorTabSpaces(mWindow->fileRows.at(mWindow->fileCursorY));
	while (mWindow->fileCursorX + spaces < mWindow->savedRenderedCursorXPos)
	{
		++mWindow->fileCursorX;
		spaces = getRenderedCursorTabSpaces(mWindow->fileRows.at(mWindow->fileCursorY));
	}
	if (mWindow->fileCursorX + spaces > mWindow->savedRenderedCursorXPos)
	{
		--mWindow->fileCursorX;
	}
	if (mWindow->fileCursorX > mWindow->fileRows.at(mWindow->fileCursorY).line.length())
	{
		mWindow->fileCursorX = mWindow->fileRows.at(mWindow->fileCursorY).line.length();
	}
}

/// <summary>
/// Fixes the rendered cursor x/y and row/column offset positions
/// </summary>
/// <param name="row">The row we are currently on</param>
void Console::fixRenderedCursorPosition(const FileHandler::Row& row)
{
	//Fixing rendered X/Col position
	mWindow->renderedCursorX = mWindow->fileCursorX;
	mWindow->renderedCursorX += getRenderedCursorTabSpaces(row);
	mWindow->colNumberToDisplay = mWindow->renderedCursorX;

	while (mWindow->renderedCursorX - mWindow->colOffset >= mWindow->cols && mWindow->renderedCursorX >= mWindow->colOffset)
	{
		++mWindow->colOffset;
	}
	while (mWindow->renderedCursorX < mWindow->colOffset)
	{
		--mWindow->colOffset;
	}
	mWindow->renderedCursorX = mWindow->renderedCursorX - mWindow->colOffset;
	if (mWindow->renderedCursorX == mWindow->cols)
	{
		--mWindow->renderedCursorX;
	}
	//Fixing rendered Y/Row position
	while (mWindow->fileCursorY - mWindow->rowOffset >= mWindow->rows && mWindow->fileCursorY >= mWindow->rowOffset)
	{
		++mWindow->rowOffset;
	}
	while (mWindow->fileCursorY < mWindow->rowOffset)
	{
		--mWindow->rowOffset;
	}
	mWindow->renderedCursorY = mWindow->fileCursorY - mWindow->rowOffset;

	if (mWindow->renderedCursorY == mWindow->rows) //renderedCursorY might be 1 too many rows down, so just bring it back one row if it is
	{
		--mWindow->renderedCursorY;
	}

	if (mWindow->updateSavedPos)
	{
		mWindow->savedRenderedCursorXPos = mWindow->renderedCursorX;
		mWindow->updateSavedPos = false;
	}
}


/// <summary>
/// Adjusts the rendered line, adding spaces in place of tabs as needed
/// Tab stops are every 8 columns, so the max possible amount of spaces to replace is 7
/// </summary>
/// <param name="renderedLine"></param>
void Console::replaceRenderedStringTabs(std::string& renderedLine)
{
	size_t lineLength = renderedLine.length();
	for (size_t i = 0; i < lineLength; ++i)
	{
		if (i >= renderedLine.length()) return;
		if (renderedLine[i] != static_cast<uint8_t>(KeyActions::KeyAction::Tab)) continue;

		renderedLine[i] = ' '; //Replace the tab character with a space
		uint8_t t = maxSpacesForTab - (i % tabSpacing);
		while (t > 0)
		{
			renderedLine.insert(renderedLine.begin() + i, ' '); //Add spaces until we reach a multiple of 8
			--t;
			++i;
		}
		if (renderedLine.length() > lineLength)
		{
			lineLength = renderedLine.length();
		}
	}
}

/// <summary>
/// Gets the amount of spaces the rendered cursor needs to be adjusted to account for tabs
/// </summary>
/// <param name="row">Row to check</param>
size_t Console::getRenderedCursorTabSpaces(const FileHandler::Row& row)
{
	size_t spacesToAdd = 0;
	for (size_t i = 0; i < mWindow->fileCursorX; ++i)
	{
		if (i > row.line.length()) return 0;

		if (row.line[i] != static_cast<uint8_t>(KeyActions::KeyAction::Tab)) continue;

		spacesToAdd += maxSpacesForTab - ((i + spacesToAdd) % tabSpacing); //Tabs are replaced with up to 8 spaces, depending on how close to a multiple of 8 the tab is
	}
	return spacesToAdd;
}

/// <summary>
/// Sets the rendered color for any highlight on screen
/// </summary>
/// <param name="rowOffset"></param>
/// <param name="colOffset"></param>
void Console::updateRenderedColor(const size_t rowOffset, const size_t colOffset)
{
	std::string normalColorMode = "\x1b[0m";
	size_t charactersToAdjust = 0; //The amount of characters to adjust for in the string position based on how many color code escape sequences have been added
	size_t prevRow = 0;
	for (const auto& highlight : mHighlights)
	{
		if (!highlight.drawColor) continue;
		if (highlight.startRow == highlight.endRow && (highlight.endCol < colOffset || highlight.startCol > colOffset + mWindow->cols)) continue;
		if (highlight.startRow > mWindow->rowOffset + mWindow->rows) return;

		std::string* renderString = &mWindow->fileRows.at(highlight.startRow).renderedLine;
		if (prevRow != highlight.startRow) charactersToAdjust = 0;

		const uint8_t color = SyntaxHighlight::color(highlight.highlightType);
		std::string colorFormat = std::format("\x1b[38;5;{}m", std::to_string(color));
		if (rowOffset > highlight.startRow)
		{
			renderString = &mWindow->fileRows.at(rowOffset).renderedLine;
			renderString->insert(0, colorFormat);
			charactersToAdjust += colorFormat.length();
			prevRow = rowOffset;
		}
		else
		{
			size_t insertPos = highlight.startCol;
			//Need to make sure the insert position is in within the rendered string
			if (insertPos < colOffset) insertPos = 0;
			else if (insertPos >= colOffset) insertPos -= colOffset;
			if (insertPos >= mWindow->cols) insertPos = mWindow->cols;

			renderString->insert(insertPos + charactersToAdjust, colorFormat);
			charactersToAdjust += colorFormat.length();
			prevRow = highlight.startRow;
		}

		size_t insertPos = highlight.endCol;
		if (insertPos >= colOffset) insertPos -= colOffset;
		else if (insertPos < colOffset) insertPos = 0;
		if (insertPos >= mWindow->cols) insertPos = mWindow->cols;
		renderString = &mWindow->fileRows.at(highlight.endRow).renderedLine;

		if (prevRow != highlight.endRow) charactersToAdjust = 0;
		renderString->insert(insertPos + charactersToAdjust, normalColorMode);
		charactersToAdjust += normalColorMode.length();

		prevRow = highlight.endRow;
	}
}

/// <summary>
/// Sets the highlight points of the rendered string if a syntax is given
/// May need more testing, but should be optimized and bug-free
/// </summary>
void Console::setHighlight()
{
	if (!SyntaxHighlight::hasSyntax()) return; //Can't highlight if there is no syntax

	std::tuple<int64_t, int64_t> offsets = SyntaxHighlight::removeOffScreenHighlights(mWindow->rowOffset, mWindow->rows, mWindow->fileCursorY);
	int64_t rowToStart = std::get<0>(offsets);
	int64_t colToStart = std::get<1>(offsets);

	size_t i = rowToStart == -1 ? mWindow->rowOffset : rowToStart;

	if (rowToStart == -1)
	{
		colToStart = 0; //If the row to start is not before the row offset, just reset the column offset since we need to check the full row anyways
	}

	while (i < mWindow->fileRows.size() && i <= mWindow->rowOffset + mWindow->rows)
	{
		if (i > mWindow->rowOffset + mWindow->rows) return;

		FileHandler::Row* row = &mWindow->fileRows.at(i); //The starting row

		size_t findPos = 0, posOffset = colToStart; //posOffset keeps track of how far into the string we are, since findPos depends on currentWord, which progressively gets smaller

		if (i < mWindow->rowOffset)
		{
			replaceRenderedStringTabs(row->renderedLine); //Make sure we get the correct column position
		}

		std::string renderedLineCopy = row->renderedLine.substr(colToStart); //Make a copy of the renderedLine so there is no dangling pointer
		std::string_view currentWord = renderedLineCopy; //Use a string_view for string parsing, as it is more efficient than constantly making copies of strings with substr()
		colToStart = 0;

		while ((findPos = currentWord.find_first_of(separators)) != std::string::npos)
		{
			row = &mWindow->fileRows.at(i); //Makes sure the correct row is always being used

			std::string_view wordToCheck = currentWord.substr(0, findPos); //The word/character sequence before the separator character

			if (!wordToCheck.empty())
			{
				SyntaxHighlight::highlightKeywordNumberCheck(wordToCheck, i, posOffset);
			}

			bool gotoNextRow = SyntaxHighlight::highlightCommentCheck(mWindow->fileRows, currentWord, row, findPos, posOffset, i);
			if(gotoNextRow)
			{
				goto nextrow;
			}
		}

		if (!currentWord.empty()) //If the last 'word' in the string isn't a separator character/comment/string
		{
			SyntaxHighlight::highlightKeywordNumberCheck(currentWord, i, posOffset);
			goto nextrow;
		}
	nextrow:
		++i;
		continue;
	}
}

//=================================================================== OS-SPECIFIC FUNCTIONS =============================================================================\\

#ifdef _WIN32
DWORD defaultMode;
#elif defined(__linux__) || defined(__APPLE__)
static termios defaultMode;
#endif

/// <summary>
/// Initializes the window and all other dependencies
/// </summary>
/// <param name="fName">The filename grabbed from argv[1]</param>
void Console::initConsole(const std::string_view& fName)
{
	FileHandler::fileName(fName);
	SyntaxHighlight::initSyntax(fName);

	mWindow = std::make_unique<Window>(Window());
	setWindowSize();

#ifdef _WIN32
	if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode)) //Try to get the default terminal settings
	{
		std::cerr << "Error retrieving current console mode";
		exit(EXIT_FAILURE);
	}
#elif defined(__linux__) || defined(__APPLE__)
	if (tcgetattr(STDOUT_FILENO, &defaultMode) == -1)
	{
		std::cerr << "Error retrieving current console mode";
		exit(EXIT_FAILURE);
	}
	signal(SIGWINCH, nullptr);
#endif

	if (!(mWindow->rawModeEnabled = enableRawInput())) //Try to enable raw mode
	{
		std::cerr << "Error enabling raw input mode";
		exit(EXIT_FAILURE);
	}

	prepRenderedString();
}

/// <summary>
/// Sets the window's row/column count
/// </summary>
/// <returns>True if size has changed, false otherwise</returns>
bool Console::setWindowSize()
{
	const size_t prevRows = mWindow->rows;
	const size_t prevCols = mWindow->cols;
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &screenInfo))
	{
		std::cerr << "Error getting console screen buffer info";
		exit(EXIT_FAILURE);
	}
	mWindow->rows = static_cast<size_t>(screenInfo.srWindow.Bottom - screenInfo.srWindow.Top) + 1;
	mWindow->cols = static_cast<size_t>(screenInfo.srWindow.Right - screenInfo.srWindow.Left) + 1;

#elif defined(__linux__) || defined(__APPLE__)
	winsize ws;
	int rows, cols;
	if (ioctl(1, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) //If getting the window size from ioctl fails
	{
		std::cout.write("\x1b[999C\x1b[999B", 12); //Move to the bottom-right corner of the screen
		std::cout.write("\x1b[6n", 4); //Report cursor position - Gets reported as ESC[rows;colsR

		std::string buf; buf.resize(32);
		uint8_t i = 0;
		while (i < buf.size())
		{
			std::cin.read(buf.data(), 1);
			if (buf[i] == 'R') break;
			++i;
		}

		if (buf[0] != static_cast<uint8_t>(KeyActions::KeyAction::Esc) || buf[1] != '[')
		{
			std::cerr << "Error getting window size";
			exit(EXIT_FAILURE);
		}

		//Working on finding a replacement for this, as this introduces possible buffer overflow risk
		if (sscanf(buf.data() + 2, "%d;%d", &rows, &cols) != 2) //If the rows/cols data wasn't reported
		{
			std::cerr << "Error getting window size";
			exit(EXIT_FAILURE);
		}
		mWindow->rows = rows; mWindow->cols = cols;
	}
	else
	{
		mWindow->cols = ws.ws_col;
		mWindow->rows = ws.ws_row;
	}
#endif
	mWindow->rows -= statusMessageRows;

	if (prevRows != mWindow->rows || prevCols != mWindow->cols) return true; //Checks if the window size has changed

	return false;
}

/// <summary>
/// Enables raw input mode by disabling specific flags
/// </summary>
/// <returns></returns>
bool Console::enableRawInput()
{
	if (mWindow->rawModeEnabled) return true;
#ifdef _WIN32
	DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT
		& ~ENABLE_ECHO_INPUT & ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT); //Disabling certain input/output modes to enable raw mode

	if (SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode))
	{
		atexit(disableRawInput); //Make sure raw input mode gets disabled if the program exits due to an error
		return true;
	}
	return false;
#elif defined(__linux__) || defined(__APPLE__)
	termios raw;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw) < 0)
	{
		return false;
	}
	atexit(disableRawInput); //Make sure raw input mode gets disabled if the program exits due to an error
	return true;
#endif
}

/// <summary>
/// Disabled raw input mode by setting the terminal back to the default mode
/// </summary>
void Console::disableRawInput()
{
#ifdef _WIN32
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
#elif defined(__linux__) || defined(__APPLE__)
	tcsetattr(STDOUT_FILENO, TCSAFLUSH, &defaultMode);
#endif
	mWindow->rawModeEnabled = false;
}