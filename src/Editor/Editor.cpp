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
* @file Editor.cpp
* @brief Provides the implementation for handling terminal output and cursor movement
* 
* Controls everything that is displayed to the editor, as well as its position relative to the file. This includes:
* Cursor position, rendered text, as well as how the cursor moves through the file. Also includes file history for undo/redo
* Essentially, this is the core piece that binds this project together.
*/

#include "Editor.hpp"

#include <iostream>
#include <tuple>
#include <limits>
#include <utility>
#include <format> //C++20 is required. MSVC/GCC-13/Clang-14/17/AppleClang-15

constexpr char MiniVersion[7] = "0.7.0a";

Editor::Window::Window(FileHandler& file) : fileCursorX(0), fileCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0), colNumberToDisplay(0), savedRenderedCursorXPos(0),
rowOffset(0), colOffset(0), dirty(false), fileRows(file.getFileContents())
{}

const Editor::Mode Editor::mode() const
{
	return mMode;
}

Editor::Editor(SyntaxHighlight syntax, FileHandler fileHandler, std::unique_ptr<IConsole> console) : mSyntax(std::move(syntax)), 
	mFile(std::move(fileHandler)), mConsole(std::move(console)), mFindLocations()
{
	mWindow = std::make_unique<Window>(Window(mFile));
	updateWindowSize();

	if (mSyntax.hasSyntax())
	{
		normalColorMode = std::format("\x1b[38;5;{}m", std::to_string(mSyntax.color(SyntaxHighlight::HighlightType::Normal)));
	}
	else
	{
		normalColorMode = "\x1b[0m";
	}
}

void Editor::clearScreen()
{
	std::string clearScreen = "\x1b[2J\x1b[3J\x1b[H"; //Clear screen, clear saved lines, and move cursor to Home (0,0)
	std::cout << clearScreen;
	std::cout.flush();
}

void Editor::prepForRender()
{
	if (mWindow->fileRows->size() > 0 && mMode != Mode::CommandMode) fixRenderedCursorPosition(mWindow->fileRows->at(mWindow->fileCursorY));
	setHighlight();
	setRenderedLineLength();
}

void Editor::setRenderedLine(const size_t startRow, const size_t endRow)
{
	for (size_t r = startRow; r <= endRow && r < mWindow->fileRows->size(); ++r)
	{
		FileHandler::Row& row = mWindow->fileRows->at(r);
		row.renderedLine = row.line;
		if (row.renderedLine.length() > 0)
		{
			replaceRenderedStringTabs(row.renderedLine);
		}
	}
}

void Editor::setRenderedLineLength()
{
	for (size_t y = mWindow->rowOffset; y < mWindow->fileRows->size() && y < mWindow->rows + mWindow->rowOffset; ++y)
	{
		FileHandler::Row& row = mWindow->fileRows->at(y);

		//Set the render string length to the lesser of the console width and the line length.
		const size_t renderedLength = std::min(static_cast<size_t>(mWindow->cols - 1), row.renderedLine.length() - mWindow->colOffset);
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

std::string Editor::renderCursor()
{
	if (mMode == Mode::CommandMode || mMode == Mode::FindMode)
	{
		mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + statusMessageRows;
	}

	size_t rowToDisplay = mWindow->renderedCursorY + 1; //Add 1 for display only. Actual rows/cols are 0-indexed internally
	size_t colToDisplay = mWindow->renderedCursorX + 1;

	std::string cursorPosition = std::format("\x1b[{};{}H", rowToDisplay, colToDisplay); //Move the cursor to this position after rendering status bar
	return cursorPosition;
}

std::string Editor::renderStatus() 
{
	const uint8_t statusRowStart = mWindow->rows + 1;
	constexpr uint8_t statusColStart = 0;
	std::string statusBuffer = std::format("\x1b[{};{}H", statusRowStart, statusColStart); //Move the cursor to this position to draw the status bar
	statusBuffer.append("\x1b[0m\x1b[0K");

	statusBuffer.append(normalColorMode);
	statusBuffer.append("\x1b[7m"); //Set to inverse color mode (white background dark text) for status row

	std::string rStatus, modeToDisplay;
	std::string status = std::format("{} - {} lines {}", mFile.fileName(), mWindow->fileRows->size(), mWindow->dirty ? "(modified)" : "");

	//Set the displayed mode and the cursor position for display
	size_t currentRowToDisplay = mWindow->rowOffset + mWindow->renderedCursorY + 1; //Add 1 for display only. Rows and cols are 0-indexed internally
	size_t currentColToDisplay = mWindow->colNumberToDisplay + 1;
	if (mMode == Mode::EditMode)
	{
		rStatus = std::format("row {}/{} col {}", currentRowToDisplay, mWindow->fileRows->size(), currentColToDisplay);
		modeToDisplay = "EDIT";
	}
	else if (mMode == Mode::ReadMode)
	{
		rStatus = std::format("row {}/{} col {}", currentRowToDisplay, mWindow->fileRows->size(), currentColToDisplay);
		modeToDisplay = "READ ONLY";
	}
	else if (mMode == Mode::CommandMode)
	{
		rStatus = "Enter command";
		modeToDisplay = "COMMAND";
	}
	else if (mMode == Mode::FindMode)
	{
		rStatus = std::format("match {}/{}", mCurrentFindPos, mFindLocations.size());
		modeToDisplay = "FIND";
	}
	
	size_t statusLength = std::min(status.length(), static_cast<size_t>(mWindow->cols));
	if (statusLength < status.length())
	{
		status.resize(statusLength);
	}

	statusBuffer.append(status);

	while (statusLength < (mWindow->cols / 2))
	{
		if ((mWindow->cols / 2) - statusLength == modeToDisplay.length() / 2)
		{
			statusBuffer.append(modeToDisplay);
			break;
		}
		else
		{
			statusBuffer.append(" ");
			++statusLength;
		}
	}
	statusLength += modeToDisplay.length();

	while (statusLength <= mWindow->cols)
	{
		if (mWindow->cols - statusLength - 1 == rStatus.length())
		{
			statusBuffer.append(rStatus);
			break;
		}
		else
		{
			statusBuffer.append(" ");
			++statusLength;
		}
	}

	if (mMode != Mode::FindMode && mMode != Mode::CommandMode)
	{
		statusBuffer.append("\n\x1b[0K");
	}

	return statusBuffer; //Send the status bar to be rendered
}

std::string Editor::renderStatusAndCursor()
{
	std::string statusAndCursorBuffer = "";
	statusAndCursorBuffer.append(renderStatus());
	statusAndCursorBuffer.append(renderCursor());
	statusAndCursorBuffer.append("\x1b[0m");

	if (mMode == Mode::CommandMode || mMode == Mode::FindMode)
	{
		statusAndCursorBuffer.append(mCommandBuffer);
	}

	return statusAndCursorBuffer;
}

void Editor::renderEndOfFile() 
{
	const char* emptyRowCharacter = "~";
	for (size_t y = mWindow->rowOffset; y < mWindow->rows + mWindow->rowOffset; ++y)
	{
		if (y >= mWindow->fileRows->size())
		{
			if (mWindow->fileRows->size() == 0 && y == mWindow->rows / 3) //If the file is empty and the current row is at 1/3 height (good display position)
			{
				std::string welcome = std::format("Mini Editor -- version {}\x1b[0K\r\n", MiniVersion);
				size_t padding = (mWindow->cols - welcome.length()) / 2;
				if (padding > 0)
				{
					mTextRenderBuffer.append(emptyRowCharacter);
					--padding;
				}
				while (padding > 0)
				{
					mTextRenderBuffer.append(" ");
					--padding;
				}
				mTextRenderBuffer.append(welcome);
			}
			else
			{
				mTextRenderBuffer.append(normalColorMode); //Make sure color mode is back to normal
				mTextRenderBuffer.append(emptyRowCharacter);
				mTextRenderBuffer.append("\x1b[0K\r\n"); //Clear the rest of the row
			}
			continue;
		}
	}
}

void Editor::refreshScreen(bool forceRedrawScreen)
{
	mMutex.lock(); //Refresh screen may be called from a separate thread

	if (forceRedrawScreen)
	{
		clearScreen();
		if (mWindow->fileRows->size() > 0) fixRenderedCursorPosition(mWindow->fileRows->at(mWindow->fileCursorY));
		else
		{
			mWindow->renderedCursorX = 0; mWindow->renderedCursorY = 0;
		}
	}

	prepForRender();
	std::string finalBufferToRender = "";

	mTextRenderBuffer = "\x1b[H"; //Move cursor to start of screen to redraw changes.

	updateRenderedColor(mWindow->rowOffset, mWindow->colOffset);
	for (size_t i = mWindow->rowOffset; i < mWindow->fileRows->size() && i < mWindow->rowOffset + mWindow->rows; ++i)
	{
		mTextRenderBuffer.append(mWindow->fileRows->at(i).renderedLine);
		mTextRenderBuffer.append("\x1b[0K\r\n");
	}

	mTextRenderBuffer.append(normalColorMode); //Make sure color mode is back to normal

	if (mWindow->rowOffset + mWindow->rows >= mWindow->fileRows->size())
	{
		renderEndOfFile();
	}
	
	mTextRenderBuffer.append("\x1b[3J"); //Remove saved lines (so there is no scroll bar)

	if (mTextRenderBuffer != mPreviousTextRenderBuffer || forceRedrawScreen) 
	{
		finalBufferToRender.append(mTextRenderBuffer);
		mPreviousTextRenderBuffer = mTextRenderBuffer;
	}

	finalBufferToRender.append(renderStatusAndCursor());
	std::cout << finalBufferToRender;
	std::cout.flush(); //Flush the buffer after rendering everything to screen

	mMutex.unlock(); //Finally unlock the mutex so main thread and secondary thread can do their thing
}

int8_t Editor::moveCursorLeftRight(const KeyActions::KeyAction key)
{
	bool isForward = (key == KeyActions::KeyAction::ArrowRight || key == KeyActions::KeyAction::CtrlArrowRight);

	if (isForward)
	{
		if (mWindow->fileCursorY == mWindow->fileRows->size() - 1 
		 && mWindow->fileCursorX == mWindow->fileRows->at(mWindow->fileCursorY).line.length()) return cursorCantMove; //Can't move any farther right if we are at the end of the file

		if (mWindow->fileCursorX == mWindow->fileRows->at(mWindow->fileCursorY).line.length())
		{
			++mWindow->fileCursorY;
			mWindow->fileCursorX = 0;
			return cursorMovedNewLine;
		}
	}
	else
	{
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0) return cursorCantMove;

		if (mWindow->fileCursorX == 0)
		{
			--mWindow->fileCursorY;
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
			return cursorMovedNewLine;
		}
	}

	return cursorMoveNormal;
}

void Editor::moveCursor(const KeyActions::KeyAction key)
{
	if (mWindow->fileRows->size() == 0) return;

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
	case KeyActions::KeyAction::ArrowLeft:
		if (returnCode == cursorMoveNormal)
		{
			--mWindow->fileCursorX;
		}
		break;
		
	case KeyActions::KeyAction::ArrowRight:
		if (returnCode == cursorMoveNormal)
		{
			++mWindow->fileCursorX;
		}
		break;

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
		if (mWindow->fileCursorY == mWindow->fileRows->size() - 1)
		{
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
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
				char charToFind = mWindow->fileRows->at(mWindow->fileCursorY).line[mWindow->fileCursorX];
				if (separators.find(charToFind) != std::string::npos) break;
			}
		} 
		break;

	case KeyActions::KeyAction::CtrlArrowRight:
		if(returnCode == cursorMoveNormal)
		{
			while (mWindow->fileCursorX < mWindow->fileRows->at(mWindow->fileCursorY).line.length())
			{
				++mWindow->fileCursorX;
				char charToFind = mWindow->fileRows->at(mWindow->fileCursorY).line[mWindow->fileCursorX];
				if (separators.find(charToFind) != std::string::npos) break;
			}
		}

		break;

	case KeyActions::KeyAction::Home:
		mWindow->fileCursorX = 0;
		break;
		
	case KeyActions::KeyAction::End:
		mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
		break;

	case KeyActions::KeyAction::CtrlHome:
		mWindow->fileCursorX = 0; mWindow->fileCursorY = 0;
		break;

	case KeyActions::KeyAction::CtrlEnd:
		mWindow->fileCursorY = mWindow->fileRows->size() - 1;
		mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
		break;

	case KeyActions::KeyAction::CtrlPageUp: //Move cursor to top of screen
		mWindow->fileCursorY -= (mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows;
		if (mWindow->fileCursorX > mWindow->fileRows->at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
		}
		break;

	case KeyActions::KeyAction::CtrlPageDown: //Move cursor to bottom of screen
		if (mWindow->fileCursorY + mWindow->rows - ((mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows) > mWindow->fileRows->size() - 1)
		{
			mWindow->fileCursorY = mWindow->fileRows->size() - 1;
		}
		else
		{
			mWindow->fileCursorY += mWindow->rows - ((mWindow->fileCursorY - mWindow->rowOffset) % mWindow->rows);
		}

		if (mWindow->fileCursorX > mWindow->fileRows->at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
		}
		break;

	default:
		break;
	}
}

void Editor::shiftRowOffset(const KeyActions::KeyAction key)
{
	if (mWindow->fileRows->size() == 0) return;

	switch (key)
	{
	case KeyActions::KeyAction::CtrlArrowDown:
		if (mWindow->rowOffset == mWindow->fileRows->size() - 1) return; //This is as far as the screen can be moved down

		++mWindow->rowOffset;
		if (mWindow->fileCursorY < mWindow->fileRows->size() && mWindow->renderedCursorY == 0) //Move the file cursor if the rendered cursor is at the top of the screen
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
		if (mWindow->fileCursorX > mWindow->fileRows->at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
		}
		break;

	case KeyActions::KeyAction::PageDown: //Shift screen offset down by 1 page worth (mWindow->rows)
		if (mWindow->fileCursorY + mWindow->rows > mWindow->fileRows->size() - 1)
		{
			if (mWindow->fileCursorY == mWindow->fileRows->size() - 1) return;

			mWindow->fileCursorY = mWindow->fileRows->size() - 1;
			mWindow->rowOffset += mWindow->fileCursorY % mWindow->rows;
		}
		else
		{
			mWindow->fileCursorY += mWindow->rows;
			mWindow->rowOffset += mWindow->rows;
		}
		if (mWindow->fileCursorX > mWindow->fileRows->at(mWindow->fileCursorY).line.length())
		{
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
		}
		break;

	default:
		break;
	}
}

void Editor::addRow()
{
	addUndoHistory(FileHistory::ChangeType::RowInserted);

	FileHandler::Row& row = mWindow->fileRows->at(mWindow->fileCursorY);

	if (mWindow->fileCursorX == row.line.length())
	{
		mWindow->fileRows->insert(mWindow->fileRows->begin() + mWindow->fileCursorY + 1, FileHandler::Row());
	}
	else if (mWindow->fileCursorX == 0)
	{
		mWindow->fileRows->insert(mWindow->fileRows->begin() + mWindow->fileCursorY, FileHandler::Row());
	}
	else
	{
		FileHandler::Row newRow;
		newRow.line = row.line.substr(mWindow->fileCursorX);
		row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.end());
		mWindow->fileRows->insert(mWindow->fileRows->begin() + mWindow->fileCursorY + 1, newRow);
	}

	mWindow->fileCursorX = 0; ++mWindow->fileCursorY;
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

void Editor::deleteRow(const size_t rowNum)
{
	if (rowNum > mWindow->fileRows->size()) return;
	mWindow->fileRows->erase(mWindow->fileRows->begin() + rowNum);
	mWindow->dirty = true;
}

void Editor::deleteChar(const KeyActions::KeyAction key)
{
	FileHandler::Row& row = mWindow->fileRows->at(mWindow->fileCursorY);
	switch (key)
	{
	case KeyActions::KeyAction::Backspace:
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0)
		{
			return;
		}

		if (mWindow->fileCursorX == 0)
		{
			addUndoHistory(FileHistory::ChangeType::RowDeleted);
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY - 1).line.length();
			mWindow->fileRows->at(mWindow->fileCursorY - 1).line.append(row.line);
			deleteRow(mWindow->fileCursorY);
			--mWindow->fileCursorY;
		}
		else
		{
			addUndoHistory(FileHistory::ChangeType::CharDeleted);
			row.line.erase(row.line.begin() + mWindow->fileCursorX - 1);
			--mWindow->fileCursorX;
		}
		break;

	case KeyActions::KeyAction::Delete:
		if (mWindow->fileCursorY == mWindow->fileRows->size() - 1 && mWindow->fileCursorX == row.line.length())
		{
			return;
		}

		if (mWindow->fileCursorX == row.line.length())
		{
			addUndoHistory(FileHistory::ChangeType::RowDeleted);
			row.line.append(mWindow->fileRows->at(mWindow->fileCursorY + 1).line);
			deleteRow(mWindow->fileCursorY + 1);
		}
		else
		{
			addUndoHistory(FileHistory::ChangeType::CharDeleted);
			row.line.erase(row.line.begin() + mWindow->fileCursorX);
		}
		break;

	case KeyActions::KeyAction::CtrlBackspace:
		if (mWindow->fileCursorX == 0 && mWindow->fileCursorY == 0)
		{
			return;
		}

		if (mWindow->fileCursorX == 0)
		{
			addUndoHistory(FileHistory::ChangeType::RowDeleted);
			mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY - 1).line.length();
			mWindow->fileRows->at(mWindow->fileCursorY - 1).line.append(row.line);
			deleteRow(mWindow->fileCursorY);
			--mWindow->fileCursorY; 
		}
		else
		{
			addUndoHistory(FileHistory::ChangeType::CharDeleted);
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
		if (mWindow->fileCursorY == mWindow->fileRows->size() - 1 && mWindow->fileCursorX == row.line.length())
		{
			return;
		}

		if (mWindow->fileCursorX == row.line.length())
		{
			addUndoHistory(FileHistory::ChangeType::RowDeleted);
			row.line.append(mWindow->fileRows->at(mWindow->fileCursorY + 1).line);
			deleteRow(mWindow->fileCursorY + 1);
		}
		else
		{
			addUndoHistory(FileHistory::ChangeType::CharDeleted);
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

void Editor::insertChar(const unsigned char c)
{
	FileHandler::Row& row = mWindow->fileRows->at(mWindow->fileCursorY);

	addUndoHistory(FileHistory::ChangeType::CharInserted);

	row.line.insert(row.line.begin() + mWindow->fileCursorX, c);
	++mWindow->fileCursorX;
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

void Editor::addUndoHistory(FileHistory::ChangeType change)
{
	FileHistory history;
	history.changeType = change;
	history.rows.push_back(mWindow->fileRows->at(mWindow->fileCursorY).line); //Only need the updated line
	if (change == FileHistory::ChangeType::RowDeleted)
	{
		history.rows.push_back(mWindow->fileRows->at(mWindow->fileCursorY - 1).line);
	}
	history.fileCursorX = mWindow->fileCursorX;
	history.fileCursorY = mWindow->fileCursorY;
	history.colOffset = mWindow->colOffset;
	history.rowOffset = mWindow->rowOffset;

	mUndoHistory.push(history);
}

void Editor::addRedoHistory(FileHistory::ChangeType change)
{
	FileHistory history;
	history.rows.push_back(mWindow->fileRows->at(mWindow->fileCursorY).line); //Only need the updated line
	if (change == FileHistory::ChangeType::RowDeleted)
	{
		history.rows.push_back(mWindow->fileRows->at(mWindow->fileCursorY - 1).line);
	}
	history.changeType = change;
	history.fileCursorX = mWindow->fileCursorX;
	history.fileCursorY = mWindow->fileCursorY;
	history.colOffset = mWindow->colOffset;
	history.rowOffset = mWindow->rowOffset;

	mRedoHistory.push(history);
}

void Editor::undoChange()
{
	if (mUndoHistory.size() == 0) return;

	FileHistory::ChangeType type;
	if (mUndoHistory.top().changeType == FileHistory::ChangeType::CharDeleted) type = FileHistory::ChangeType::CharInserted;
	else if (mUndoHistory.top().changeType == FileHistory::ChangeType::CharInserted) type = FileHistory::ChangeType::CharDeleted;
	else if (mUndoHistory.top().changeType == FileHistory::ChangeType::RowDeleted) type = FileHistory::ChangeType::RowInserted;
	else type = FileHistory::ChangeType::RowDeleted;

	addRedoHistory(type);

	if (mUndoHistory.top().changeType == FileHistory::ChangeType::RowInserted)
	{
		mWindow->fileRows->erase(mWindow->fileRows->begin() + mUndoHistory.top().fileCursorY + 1);
	}
	else if (mUndoHistory.top().changeType == FileHistory::ChangeType::RowDeleted)
	{
		mWindow->fileRows->insert(mWindow->fileRows->begin() + mUndoHistory.top().fileCursorY - 1, FileHandler::Row(mUndoHistory.top().rows.at(1)));
	}
	mWindow->fileRows->at(mUndoHistory.top().fileCursorY).line = mUndoHistory.top().rows.at(0);

	mWindow->fileCursorX = mUndoHistory.top().fileCursorX;
	mWindow->fileCursorY = mUndoHistory.top().fileCursorY;
	mWindow->colOffset = mUndoHistory.top().colOffset;
	mWindow->rowOffset = mUndoHistory.top().rowOffset;

	mUndoHistory.pop();
}

void Editor::redoChange()
{
	if (mRedoHistory.size() == 0) return;

	FileHistory::ChangeType type;
	if (mRedoHistory.top().changeType == FileHistory::ChangeType::CharDeleted) type = FileHistory::ChangeType::CharInserted;
	else if (mRedoHistory.top().changeType == FileHistory::ChangeType::CharInserted) type = FileHistory::ChangeType::CharDeleted;
	else if (mRedoHistory.top().changeType == FileHistory::ChangeType::RowDeleted) type = FileHistory::ChangeType::RowInserted;
	else type = FileHistory::ChangeType::RowDeleted;

	addUndoHistory(type);

	if (mRedoHistory.top().changeType == FileHistory::ChangeType::RowInserted)
	{
		mWindow->fileRows->erase(mWindow->fileRows->begin() + mRedoHistory.top().fileCursorY + 1);
	}
	else if (mRedoHistory.top().changeType == FileHistory::ChangeType::RowDeleted)
	{
		mWindow->fileRows->insert(mWindow->fileRows->begin() + mRedoHistory.top().fileCursorY - 1, FileHandler::Row(mRedoHistory.top().rows.at(1)));
	}
	mWindow->fileRows->at(mRedoHistory.top().fileCursorY).line = mRedoHistory.top().rows.at(0);

	mWindow->fileCursorX = mRedoHistory.top().fileCursorX;
	mWindow->fileCursorY = mRedoHistory.top().fileCursorY;
	mWindow->colOffset = mRedoHistory.top().colOffset;
	mWindow->rowOffset = mRedoHistory.top().rowOffset;

	mRedoHistory.pop();
}

const bool Editor::isDirty() const
{
	return mWindow->dirty;
}

void Editor::save()
{
	mFile.saveFile();
	mWindow->dirty = false;
}

void Editor::enableCommandMode()
{
	mMode = Mode::CommandMode;
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + statusMessageRows;
}

void Editor::enableFindMode()
{
	mMode = Mode::FindMode;
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + statusMessageRows;
}

void Editor::enableEditMode()
{
	if (mWindow->fileRows->size() == 0)
	{
		mWindow->fileRows->push_back(FileHandler::Row());
	}
	mMode = Mode::EditMode;
}

void Editor::enableReadMode()
{
	mMode = Mode::ReadMode;
	mFindLocations.clear();
}

void Editor::enableExitMode()
{
	mMode = Mode::ExitMode;
	mConsole->disableRawInput();
}

void Editor::setCursorLinePosition()
{
	if (mWindow->renderedCursorX > mWindow->fileRows->at(mWindow->fileCursorY).renderedLine.length())
	{
		mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
		return;
	}
	mWindow->fileCursorX = 0;
	size_t spaces = getRenderedTabSpaces(mWindow->fileRows->at(mWindow->fileCursorY), mWindow->fileCursorX);
	while (mWindow->fileCursorX + spaces < mWindow->savedRenderedCursorXPos)
	{
		++mWindow->fileCursorX;
		spaces = getRenderedTabSpaces(mWindow->fileRows->at(mWindow->fileCursorY), mWindow->fileCursorX);
	}
	if (mWindow->fileCursorX + spaces > mWindow->savedRenderedCursorXPos)
	{
		--mWindow->fileCursorX;
	}
	if (mWindow->fileCursorX > mWindow->fileRows->at(mWindow->fileCursorY).line.length())
	{
		mWindow->fileCursorX = mWindow->fileRows->at(mWindow->fileCursorY).line.length();
	}
}

void Editor::fixRenderedCursorPosition(const FileHandler::Row& row)
{
	//Fixing rendered X/Col position
	mWindow->renderedCursorX = mWindow->fileCursorX;
	mWindow->renderedCursorX += getRenderedTabSpaces(row, mWindow->fileCursorX);
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

void Editor::replaceRenderedStringTabs(std::string& renderedLine)
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

const size_t Editor::getRenderedTabSpaces(const FileHandler::Row& row, size_t endPos) const
{
	size_t spacesToAdd = 0;
	for (size_t i = 0; i < endPos; ++i)
	{
		if (i > row.line.length()) return 0;

		if (row.line[i] != static_cast<uint8_t>(KeyActions::KeyAction::Tab)) continue;

		spacesToAdd += maxSpacesForTab - ((i + spacesToAdd) % tabSpacing); //Tabs are replaced with up to 8 spaces, depending on how close to a multiple of 8 the tab is
	}
	return spacesToAdd;
}

size_t Editor::adjustSyntaxHighlightLocations(const size_t adjustmentsMade, const FindString::FindLocation& findLocation, const size_t findColorLength)
{
	if (!mSyntax.hasSyntax()) return 0;

	size_t adjustmentToMake = 0;
	const size_t highlightsSize = mSyntax.highlights().size();
	for (size_t i = 0; i < highlightsSize; ++i)
	{
		SyntaxHighlight::HighlightLocation* highlight = &mSyntax.highlights().at(i);
		if (highlight->startRow > findLocation.row) break;

		if (highlight->startRow == findLocation.row || highlight->endRow == findLocation.row)
		{
			if (highlight->startRow == findLocation.row)
			{
				if (highlight->startCol + highlight->startPosAdjustment >= findLocation.startCol + adjustmentsMade)
				{
					if (highlight->startCol + highlight->startPosAdjustment <= findLocation.startCol + findLocation.length + adjustmentsMade)
					{
						highlight->startPosAdjustment += findColorLength;
						adjustmentToMake = (adjustmentToMake == 0) ? findColorLength : adjustmentToMake;
					}
					else
					{
						highlight->startPosAdjustment += findColorLength + normalBackgroundColor.length();
						adjustmentToMake = findColorLength + normalBackgroundColor.length();
					}
				}
			}
			if (highlight->endRow == findLocation.row)
			{
				if (highlight->endCol + highlight->endPosAdjustment >= findLocation.startCol + adjustmentsMade)
				{
					if (highlight->endCol + highlight->endPosAdjustment <= findLocation.startCol + findLocation.length + adjustmentsMade)
					{
						highlight->endPosAdjustment += findColorLength;
						adjustmentToMake = (adjustmentToMake == 0) ? findColorLength : adjustmentToMake;
					}
					else
					{
						highlight->endPosAdjustment += findColorLength + normalBackgroundColor.length();
						adjustmentToMake = findColorLength + normalBackgroundColor.length();
					}
				}
			}
			continue;
		}

	}

	return adjustmentToMake;
}

void Editor::updateRenderedColor(const size_t rowOffset, const size_t colOffset)
{
	size_t charactersToAdjust = 0; //The amount of characters to adjust for in the string position based on how many color code escape sequences have been added
	size_t prevRow = 0;

	if (mMode == Mode::FindMode)
	{
		size_t syntaxToFindAdjustmentsMade = 0;
		constexpr uint8_t findColorId = 237;
		constexpr uint8_t currentFindColorId = 102;
		for (size_t i = 0; i < mFindLocations.size(); ++i)
		{
			const FindString::FindLocation& findLocation = mFindLocations.at(i);
			if (findLocation.row < rowOffset || findLocation.startCol >= mWindow->cols + colOffset || findLocation.startCol + findLocation.length < colOffset) continue;
			if (findLocation.row >= rowOffset + mWindow->rows) break;

			const uint8_t currentColorId = (i == mCurrentFindPos - 1) ? currentFindColorId : findColorId;
			std::string findLocationColor = std::format("\x1b[48;5;{}m", std::to_string(currentColorId));
			std::string* renderString = &mWindow->fileRows->at(findLocation.row).renderedLine;

			if (prevRow != findLocation.row)
			{
				charactersToAdjust = 0;
				syntaxToFindAdjustmentsMade = 0;
			}

			size_t insertPos = findLocation.startCol;
			if (insertPos < colOffset) insertPos = 0;
			else if (insertPos >= colOffset) insertPos -= colOffset;

			insertPos += charactersToAdjust;
			if (insertPos >= renderString->length()) insertPos = renderString->length();

			renderString->insert(insertPos, findLocationColor);
			charactersToAdjust += findLocationColor.length();

			insertPos = findLocation.startCol + findLocation.length;
			if (insertPos >= colOffset) insertPos -= colOffset;

			insertPos += charactersToAdjust;
			if (insertPos >= renderString->length()) insertPos = renderString->length();

			renderString->insert(insertPos, normalBackgroundColor);

			charactersToAdjust += normalBackgroundColor.length();
			prevRow = findLocation.row;

			syntaxToFindAdjustmentsMade += adjustSyntaxHighlightLocations(syntaxToFindAdjustmentsMade, findLocation, findLocationColor.length());
		}
	}

	if (!mSyntax.hasSyntax()) return;
	charactersToAdjust = 0;
	prevRow = 0;

	for (const auto& highlight : mSyntax.highlights())
	{
 		if (!highlight.drawColor) continue;
		if (highlight.startRow == highlight.endRow && highlight.endCol < colOffset) continue;
		if (highlight.startRow >= mWindow->rowOffset + mWindow->rows) break;

		std::string* renderString = &mWindow->fileRows->at(highlight.startRow).renderedLine;
		if (prevRow != highlight.startRow) charactersToAdjust = 0;

		const uint8_t color = mSyntax.color(highlight.highlightType);
		std::string colorFormat = std::format("\x1b[38;5;{}m", std::to_string(color));

		if (rowOffset > highlight.startRow)
		{
			renderString = &mWindow->fileRows->at(rowOffset).renderedLine;
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

			insertPos += charactersToAdjust + highlight.startPosAdjustment;
			if (insertPos >= renderString->length()) insertPos = renderString->length();

			renderString->insert(insertPos, colorFormat);
			charactersToAdjust += colorFormat.length();
			
			prevRow = highlight.startRow;
		}

		size_t insertPos = highlight.endCol;
		if (insertPos >= colOffset) insertPos -= colOffset;
		else if (insertPos < colOffset) insertPos = 0;
		if (insertPos >= mWindow->cols) insertPos = mWindow->cols - 1;

		if (prevRow != highlight.endRow) charactersToAdjust = 0;
		insertPos += charactersToAdjust + highlight.endPosAdjustment;

		renderString = &mWindow->fileRows->at(highlight.endRow).renderedLine;
		if (insertPos >= renderString->length()) insertPos = renderString->length();


		renderString->insert(insertPos, normalColorMode);
		charactersToAdjust += normalColorMode.length();

		prevRow = highlight.endRow;
	}
}

void Editor::setHighlight()
{
	if (!mSyntax.hasSyntax()) return; //Can't highlight if there is no syntax

	std::tuple<size_t, size_t, size_t> offsets = mSyntax.removeOffScreenHighlights(mWindow->rowOffset, mWindow->rows, mWindow->fileCursorY);
	size_t rowToStart = std::get<0>(offsets);
	size_t colToStart = std::get<1>(offsets);

	size_t i = std::min(mWindow->rowOffset, rowToStart);

	if (rowToStart == std::numeric_limits<size_t>::max())
	{
		colToStart = 0; //If the row to start is not before the row offset, just reset the column offset since we need to check the full row anyways
	}

	size_t rowToEnd = std::get<2>(offsets) == std::numeric_limits<size_t>::max() ? mWindow->rows + mWindow->rowOffset : std::max(mWindow->rows + mWindow->rowOffset, std::get<2>(offsets));
	rowToEnd += 1; //Add 1 to account for new lines
	setRenderedLine(i, rowToEnd);

	while (i < mWindow->fileRows->size() && i <= mWindow->rowOffset + mWindow->rows)
	{
		FileHandler::Row* row = &mWindow->fileRows->at(i); //The starting row

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
			if (findPos >= mWindow->colOffset + mWindow->cols) goto nextrow;

			row = &mWindow->fileRows->at(i); //Makes sure the correct row is always being used

			std::string_view wordToCheck = currentWord.substr(0, findPos); //The word/character sequence before the separator character

			if (!wordToCheck.empty())
			{
				mSyntax.highlightKeywordNumberCheck(wordToCheck, i, posOffset);
			}

			bool gotoNextRow = mSyntax.highlightCommentCheck(*mWindow->fileRows, currentWord, row, findPos, posOffset, i);
			if(gotoNextRow)
			{
				goto nextrow;
			}
		}

		if (!currentWord.empty()) //If the last 'word' in the string isn't a separator character/comment/string
		{
			mSyntax.highlightKeywordNumberCheck(currentWord, i, posOffset);
			goto nextrow;
		}

	nextrow:
		++i;
		continue;
	}
}

void Editor::updateWindowSize()
{
	IConsole::WindowSize windowSize = mConsole->getWindowSize();
	mWindow->rows = windowSize.rows - statusMessageRows;
	mWindow->cols = windowSize.cols;
}

void Editor::updateCommandBuffer(const std::string& command)
{
	mCommandBuffer = command;
}

void Editor::findString(const std::string& findString)
{
	mFindLocations = FindString::find(findString, *mWindow->fileRows);
	for (auto& location : mFindLocations)
	{
		const size_t tabs = getRenderedTabSpaces(mWindow->fileRows->at(location.row), location.startCol);
		location.startCol += tabs;
	}
	mCurrentFindPos = (mFindLocations.size() > 0) ? 1 : 0;
}

void Editor::moveCursorToFind(const KeyActions::KeyAction key)
{
	switch (key)
	{
	case KeyActions::KeyAction::ArrowLeft:
	case KeyActions::KeyAction::ArrowUp:
		if (mCurrentFindPos == 1 && mFindLocations.size() > 0) mCurrentFindPos = mFindLocations.size();
		else if (mCurrentFindPos == 0) return;
		else --mCurrentFindPos;

		break;

	case KeyActions::KeyAction::ArrowDown:
	case KeyActions::KeyAction::ArrowRight:
	case KeyActions::KeyAction::Enter:
		if (mCurrentFindPos == mFindLocations.size() && mFindLocations.size() > 0) mCurrentFindPos = 1;
		else if (mFindLocations.size() == 0) return;
		else ++mCurrentFindPos;

		break;
	}

	mWindow->fileCursorY = mFindLocations.at(mCurrentFindPos - 1).row;
	mWindow->fileCursorX = mFindLocations.at(mCurrentFindPos - 1).filePos;
	if (mFindLocations.at(mCurrentFindPos - 1).startCol + mFindLocations.at(mCurrentFindPos - 1).length >= mWindow->colOffset + mWindow->cols)
	{
		mWindow->colOffset = mFindLocations.at(mCurrentFindPos - 1).startCol + mFindLocations.at(mCurrentFindPos - 1).length - mWindow->cols + 1;
	}
}