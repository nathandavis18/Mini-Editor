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

#include <tuple>
#include <limits>
#include <utility>
#include <format> //C++20 is required. MSVC/GCC-13/Clang-14/17/AppleClang-15
#include <cmath>


Editor::Window::Window(FileHandler& file) : fileCursorX(0), fileCursorY(0), cols(0), rows(0), renderedCursorX(0), renderedCursorY(0), colNumberToDisplay(0), savedRenderedCursorXPos(0),
rowOffset(0), colOffset(0), dirty(false), fileRows(file.getFileContents())
{}

Editor::ChangeHistory::ChangeHistory(ChangeHistory::ChangeType change, const Window& window) :
	changeType(change), fileCursorX(window.fileCursorX), fileCursorY(window.fileCursorY), rowOffset(window.rowOffset), colOffset(window.colOffset)
{}

const Editor::Mode Editor::mode() const
{
	return mMode;
}

Editor::Editor(SyntaxHighlight syntax, FileHandler fileHandler, std::unique_ptr<IConsole> console, Renderer r) : mSyntax(std::move(syntax)), 
	mFile(std::move(fileHandler)), mConsole(std::move(console)), mRenderer(r)
{
	mWindow = std::make_unique<Window>(Window(mFile));
	updateWindowSize();

	if (mSyntax.hasSyntax())
	{
		mNormalColorMode = std::format("\x1b[38;5;{}m", std::to_string(mSyntax.color(SyntaxHighlight::HighlightType::Normal)));
	}
	else
	{
		constexpr uint8_t white = 15;
		mNormalColorMode = std::format("\x1b[38;5;{}m", std::to_string(white));
	}
}

void Editor::prepForRender()
{
	if (mWindow->fileRows->size() > 0 && !(mMode == Mode::CommandMode || mMode == Mode::FindInputMode || mMode == Mode::ReplaceInputMode)) fixRenderedCursorPosition(mWindow->fileRows->at(mWindow->fileCursorY));

	size_t rowToStart = mWindow->rowOffset;
	size_t colToStart = 0;
	size_t rowToEnd = mWindow->rowOffset + mWindow->rows;
	if (mSyntax.hasSyntax())
	{

		std::tuple<size_t, size_t, size_t> offsets = mSyntax.removeOffScreenHighlights(mWindow->rowOffset, mWindow->rows, mWindow->fileCursorY);

		rowToStart = std::get<0>(offsets);

		if (rowToStart != std::numeric_limits<size_t>::max()) colToStart = std::get<1>(offsets);

		rowToStart = std::min(mWindow->rowOffset, rowToStart);

		size_t rowToEnd = (std::get<2>(offsets) == std::numeric_limits<size_t>::max()) ? mWindow->rows + mWindow->rowOffset : std::max(mWindow->rows + mWindow->rowOffset, std::get<2>(offsets));
		rowToEnd += 1; //Add 1 to account for new lines
	}
	setRenderedLine(rowToStart, rowToEnd);
	setHighlightLocations(rowToStart, colToStart);
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

void Editor::prepStatusForRender()
{
	std::string mode;
	if (mMode == Mode::ReadMode)											mode = "READ ONLY";
	else if (mMode == Mode::EditMode)										mode = "EDIT";
	else if (mMode == Mode::CommandMode)									mode = "COMMAND";
	else if (mMode == Mode::FindInputMode || mMode == Mode::FindMode)		mode = "FIND";
	else if (mMode == Mode::ReplaceInputMode || mMode == Mode::ReplaceMode) mode = "REPLACE";

	std::string rStatus;
	if (mMode == Mode::ReadMode || mMode == Mode::EditMode)
	{
		rStatus = std::format("row {}/{} col {}", mWindow->fileCursorY + 1, mWindow->fileRows->size(), mWindow->colNumberToDisplay + 1);
	}
	else if (mMode == Mode::CommandMode)
	{
		rStatus = "Enter Command";
	}
	else if (mMode == Mode::FindInputMode || mMode == Mode::ReplaceInputMode || mMode == Mode::FindMode || mMode == Mode::ReplaceMode)
	{
		size_t findPosToDisplay = mCurrentFindPos + 1;
		if (mFindLocations.empty()) findPosToDisplay = 0;
		rStatus = std::format("match {}/{}", findPosToDisplay, mFindLocations.size());
	}

	mRenderer.setStatusBuffer(mWindow->rows + 1, mWindow->dirty, mFile.fileName(), mWindow->fileRows->size(), mWindow->fileCursorY + 1, mWindow->colNumberToDisplay + 1, mode, rStatus, mWindow->cols);
}

void Editor::refreshScreen(bool forceRedrawScreen)
{
	mMutex.lock(); //Refresh screen may be called from a separate thread

	if (forceRedrawScreen)
	{
		mRenderer.clearScreen();
		if (mWindow->fileRows->empty())
		{
			mWindow->renderedCursorX = 0;
			mWindow->renderedCursorY = 0;
		}
		else fixRenderedCursorPosition(mWindow->fileRows->at(mWindow->fileCursorY));
	}

	prepForRender();
	updateRenderedColor();

	for (size_t i = mWindow->rowOffset; i < mWindow->fileRows->size() && i < mWindow->rowOffset + mWindow->rows; ++i)
	{
		mRenderer.addRenderedLineToBuffer(mWindow->fileRows->at(i).renderedLine);
	}

	if (mWindow->rowOffset + mWindow->rows > mWindow->fileRows->size())
	{
		const uint16_t rowsToEnter = mWindow->rowOffset + mWindow->rows - mWindow->fileRows->size() + 1;
		mRenderer.addEndOfFileToBuffer(rowsToEnter, mWindow->cols, mWindow->fileRows->empty());
	}

	prepStatusForRender();

	bool renderCommandBuffer = false;
	if (mMode == Mode::CommandMode || mMode == Mode::FindInputMode || mMode == Mode::ReplaceInputMode || mMode == Mode::FindMode || mMode == Mode::ReplaceMode)
	{
		renderCommandBuffer = true;
		const uint16_t commandBufferRow = mWindow->rows + statusMessageRows;
		mRenderer.setCommandBuffer(mCommandBuffer, commandBufferRow);
		mWindow->renderedCursorY = commandBufferRow;
		mWindow->renderedCursorX = mCommandBuffer.length() - std::string("\r\x1b[0K").length();
	}

	mRenderer.setCursorBuffer(mWindow->renderedCursorY + 1, mWindow->renderedCursorX + 1);
	mRenderer.renderScreen(forceRedrawScreen, renderCommandBuffer);

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
	clearRedoHistory();

	addUndoHistory(ChangeHistory::ChangeType::RowInserted);

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

void Editor::deleteRow(const size_t fileCursor, const size_t rowNumToAppend)
{
	if (fileCursor >= mWindow->fileRows->size() || rowNumToAppend >= mWindow->fileRows->size()) return;

	addUndoHistory(ChangeHistory::ChangeType::RowDeleted, rowNumToAppend - mWindow->fileCursorY);

	mWindow->fileCursorX = mWindow->fileRows->at(fileCursor).line.length();
	mWindow->fileCursorY = fileCursor;

	const FileHandler::Row& row = mWindow->fileRows->at(rowNumToAppend);
	mWindow->fileRows->at(fileCursor).line.append(row.line);
	mWindow->fileRows->erase(mWindow->fileRows->begin() + fileCursor + 1);
}

void Editor::deleteChar(const KeyActions::KeyAction key)
{
	clearRedoHistory();

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
			deleteRow(mWindow->fileCursorY - 1, mWindow->fileCursorY);
		}
		else
		{
			addUndoHistory(ChangeHistory::ChangeType::CharDeleted, -1);
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
			deleteRow(mWindow->fileCursorY, mWindow->fileCursorY + 1);
		}
		else
		{
			addUndoHistory(ChangeHistory::ChangeType::CharDeleted, 1);
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
			deleteRow(mWindow->fileCursorY - 1, mWindow->fileCursorY);
		}
		else
		{
			int16_t charsDeleted = 0;
			size_t findPos;
			if ((findPos = row.line.substr(0, mWindow->fileCursorX).find_last_of(separators)) == std::string::npos) //Delete everything in the row to the beginning
			{
				charsDeleted = mWindow->fileCursorX;
				addUndoHistory(ChangeHistory::ChangeType::CharDeleted, -charsDeleted);
				row.line.erase(row.line.begin(), row.line.begin() + mWindow->fileCursorX);
				mWindow->fileCursorX = 0;
			}
			else if (findPos == mWindow->fileCursorX - 1)
			{
				deleteChar(KeyActions::KeyAction::Backspace); //Delete just the separator
				break;
			}
			else
			{
				charsDeleted = mWindow->fileCursorX - findPos + 1;
				addUndoHistory(ChangeHistory::ChangeType::CharDeleted, -charsDeleted);
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
			deleteRow(mWindow->fileCursorY, mWindow->fileCursorY + 1);
		}
		else
		{
			int16_t charsDeleted = 0;
			size_t findPos;
			if ((findPos = row.line.substr(mWindow->fileCursorX).find_first_of(separators)) == std::string::npos) //Delete everything in the row to the beginning
			{
				charsDeleted = row.line.length() - mWindow->fileCursorX;
				addUndoHistory(ChangeHistory::ChangeType::CharDeleted, charsDeleted);
				row.line.erase(row.line.begin() + mWindow->fileCursorX, row.line.end());
			}
			else if (findPos == 0)
			{
				deleteChar(KeyActions::KeyAction::Delete); //Delete just the separator
				break;
			}
			else
			{
				charsDeleted = findPos - mWindow->fileCursorX;
				addUndoHistory(ChangeHistory::ChangeType::CharDeleted, charsDeleted);
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
	clearRedoHistory();

	FileHandler::Row& row = mWindow->fileRows->at(mWindow->fileCursorY);

	addUndoHistory(ChangeHistory::ChangeType::CharInserted);

	row.line.insert(row.line.begin() + mWindow->fileCursorX, c);
	++mWindow->fileCursorX;
	mWindow->dirty = true;
	mWindow->updateSavedPos = true;
}

void Editor::clearRedoHistory()
{
	while (mRedoCounter > 0)
	{
		mFileHistory.pop_back();
		--mRedoCounter;
	}
}

void Editor::addUndoHistory(ChangeHistory::ChangeType change, const int16_t offset)
{
	ChangeHistory history(change, *mWindow);
	if (change == ChangeHistory::ChangeType::CharInserted)
	{
		history.rowChanged = mWindow->fileCursorY;
		history.colChanged = mWindow->fileCursorX;
	}
	else if (change == ChangeHistory::ChangeType::CharDeleted)
	{
		history.rowChanged = mWindow->fileCursorY;
		history.colChanged = mWindow->fileCursorX;
		if (offset < 0) history.colChanged += offset;
		history.changeMade = mWindow->fileRows->at(history.rowChanged).line.substr(history.colChanged, std::abs(offset));
	}
	else if (change == ChangeHistory::ChangeType::RowInserted)
	{
		history.rowChanged = mWindow->fileCursorY;
		history.colChanged = mWindow->fileCursorX;
		history.changeMade = mWindow->fileRows->at(history.rowChanged).line.substr(mWindow->fileCursorX);
	}
	else if (change == ChangeHistory::ChangeType::RowDeleted)
	{
		history.rowChanged = mWindow->fileCursorY + offset;
		history.colChanged = mWindow->fileCursorX;
		history.changeMade = mWindow->fileRows->at(history.rowChanged).line;
		history.prevLineLength = mWindow->fileRows->at(history.rowChanged - 1).line.length();
	}
	mFileHistory.push_front(std::move(history));
}

Editor::ChangeHistory::ChangeType Editor::reverseChangeType(ChangeHistory::ChangeType current)
{
	if (current == ChangeHistory::ChangeType::CharInserted)
	{
		return ChangeHistory::ChangeType::CharDeleted;
	}
	else if (current == ChangeHistory::ChangeType::CharDeleted)
	{
		return ChangeHistory::ChangeType::CharInserted;
	}
	else if (current == ChangeHistory::ChangeType::RowInserted)
	{
		return ChangeHistory::ChangeType::RowDeleted;
	}
	else if (current == ChangeHistory::ChangeType::RowDeleted)
	{
		return ChangeHistory::ChangeType::RowInserted;
	}
	return ChangeHistory::ChangeType::None;
}

void Editor::addUndoHistory(ChangeHistory history)
{
	history.changeType = reverseChangeType(history.changeType);
	mFileHistory.push_front(history);
}

void Editor::addRedoHistory(ChangeHistory history)
{
	history.changeType = reverseChangeType(history.changeType);
	if (history.changeType == ChangeHistory::ChangeType::CharDeleted)
	{
		history.changeMade = mWindow->fileRows->at(history.rowChanged).line.substr(history.colChanged, 1);
	}
	else if (history.changeType == ChangeHistory::ChangeType::RowDeleted)
	{
		history.prevLineLength = mWindow->fileRows->at(history.rowChanged).line.length();
	}
	mFileHistory.push_back(history);
	++mRedoCounter;
}

void Editor::undoChange()
{
	if (mFileHistory.size() == 0 || mFileHistory.size() == mRedoCounter) return; //If there is no history, or if there is only redo history

	const ChangeHistory& undo = mFileHistory.front();
	addRedoHistory(undo);

	mWindow->fileCursorX = undo.fileCursorX;
	mWindow->fileCursorY = undo.fileCursorY;
	mWindow->rowOffset = undo.rowOffset;
	mWindow->colOffset = undo.colOffset;

	if (undo.changeType == ChangeHistory::ChangeType::CharInserted)
	{
		mWindow->fileRows->at(undo.rowChanged).line.erase(undo.colChanged, 1);
	}
	else if (undo.changeType == ChangeHistory::ChangeType::CharDeleted)
	{
		mWindow->fileRows->at(undo.rowChanged).line.insert(undo.colChanged, undo.changeMade);
	}
	else if (undo.changeType == ChangeHistory::ChangeType::RowInserted)
	{
		mWindow->fileRows->at(undo.rowChanged).line.insert(undo.colChanged, undo.changeMade);
		mWindow->fileRows->erase(mWindow->fileRows->begin() + undo.rowChanged + 1);
	}
	else if (undo.changeType == ChangeHistory::ChangeType::RowDeleted)
	{
		mWindow->fileRows->insert(mWindow->fileRows->begin() + undo.rowChanged, FileHandler::Row(undo.changeMade));
		mWindow->fileRows->at(undo.rowChanged - 1).line.resize(undo.prevLineLength);
	}

	mFileHistory.pop_front();
}

void Editor::redoChange()
{
	if (mRedoCounter == 0) return;

	const ChangeHistory& redo = mFileHistory.back();
	addUndoHistory(redo);

	mWindow->fileCursorX = redo.colChanged;
	mWindow->fileCursorY = redo.fileCursorY;
	mWindow->rowOffset = redo.rowOffset;
	mWindow->colOffset = redo.colOffset;

	if (redo.changeType == ChangeHistory::ChangeType::CharInserted)
	{
		mWindow->fileRows->at(redo.rowChanged).line.erase(redo.colChanged, redo.changeMade.length());
	}
	else if (redo.changeType == ChangeHistory::ChangeType::CharDeleted)
	{
		mWindow->fileRows->at(redo.rowChanged).line.insert(redo.colChanged, redo.changeMade);
		if (redo.fileCursorX == redo.colChanged) ++mWindow->fileCursorX;
	}
	else if (redo.changeType == ChangeHistory::ChangeType::RowInserted)
	{
		mWindow->fileRows->at(redo.rowChanged - 1).line.insert(redo.prevLineLength, redo.changeMade);
		mWindow->fileRows->erase(mWindow->fileRows->begin() + redo.rowChanged);
		mWindow->fileCursorY = redo.rowChanged - 1;
		mWindow->fileCursorX = redo.prevLineLength;
	}
	else if (redo.changeType == ChangeHistory::ChangeType::RowDeleted)
	{
		mWindow->fileRows->insert(mWindow->fileRows->begin() + redo.rowChanged + 1, FileHandler::Row(redo.changeMade));
		mWindow->fileRows->at(redo.rowChanged).line.resize(redo.prevLineLength);
		mWindow->fileCursorX = 0;
		++mWindow->fileCursorY;
	}

	mFileHistory.pop_back();
	--mRedoCounter;
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

void Editor::enableFindInputMode()
{
	mMode = Mode::FindInputMode;
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + statusMessageRows;
}

void Editor::enableFindMode()
{
	mMode = Mode::FindMode;
}

void Editor::enableReplaceInputMode()
{
	mMode = Mode::ReplaceInputMode;
	mWindow->renderedCursorX = 0; mWindow->renderedCursorY = mWindow->rows + statusMessageRows;
}

void Editor::enableReplaceMode()
{
	mMode = Mode::ReplaceMode;
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
	for (size_t i = 0; i < renderedLine.length(); ++i)
	{
		if (i >= renderedLine.length()) return;	
		if (renderedLine[i] != static_cast<uint8_t>(KeyActions::KeyAction::Tab)) continue;

		renderedLine[i] = ' '; //Replace the tab character with a space
		uint8_t t = maxSpacesForTab - (i % tabSpacing);
		if (t > 0)
		{
			renderedLine.insert(i, t, ' ');
			i += t;
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

size_t Editor::adjustSyntaxHighlightLocations(const size_t adjustmentsMade, const FindAndReplace::FindLocation& findLocation, const size_t findColorLength)
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
		}
	}

	return adjustmentToMake;
}

void Editor::addFindLocationColor(const size_t rowOffset, const size_t colOffset)
{
	size_t charactersToAdjust = 0; //The amount of characters to adjust for in the string position based on how many color code escape sequences have been added
	size_t prevRow = 0;
	size_t syntaxToFindAdjustmentsMade = 0;
	constexpr uint8_t findColorId = 237;
	constexpr uint8_t currentFindColorId = 102;

	for (size_t i = 0; i < mFindLocations.size(); ++i)
	{
		const FindAndReplace::FindLocation& findLocation = mFindLocations.at(i);
		if (findLocation.row < rowOffset || findLocation.startCol >= mWindow->cols + colOffset || findLocation.startCol + findLocation.length < colOffset) continue;
		if (findLocation.row >= rowOffset + mWindow->rows) break;

		const uint8_t currentColorId = (i == mCurrentFindPos) ? currentFindColorId : findColorId;
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

void Editor::addSyntaxHighlightColor(const size_t rowOffset, const size_t colOffset)
{
	size_t charactersToAdjust = 0; //The amount of characters to adjust for in the string position based on how many color code escape sequences have been added
	size_t prevRow = 0;

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


		renderString->insert(insertPos, mNormalColorMode);
		charactersToAdjust += mNormalColorMode.length();

		prevRow = highlight.endRow;
	}
}

void Editor::updateRenderedColor()
{
	if (mMode == Mode::FindInputMode || mMode == Mode::ReplaceInputMode || mMode == Mode::FindMode || mMode == Mode::ReplaceMode)
	{
		addFindLocationColor(mWindow->rowOffset, mWindow->colOffset);
	}
	if (mSyntax.hasSyntax())
	{
		addSyntaxHighlightColor(mWindow->rowOffset, mWindow->colOffset);
	}
}

void Editor::setHighlightLocations(const size_t rowToStart, size_t colToStart)
{
	if (!mSyntax.hasSyntax()) return; //Can't highlight if there is no syntax

	for(size_t i = rowToStart; i < mWindow->fileRows->size() && i < mWindow->rowOffset + mWindow->rows; ++i)
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
	mFindLocations = FindAndReplace::find(findString, *mWindow->fileRows);
	for (auto& location : mFindLocations)
	{
		const size_t tabs = getRenderedTabSpaces(mWindow->fileRows->at(location.row), location.startCol);
		location.startCol += tabs;
	}
	mCurrentFindPos = 0;

	if (mFindLocations.empty())
	{
		enableReadMode();
		return;
	}

	constexpr uint8_t startLocation = 0;
	const FindAndReplace::FindLocation& findLocation = mFindLocations.at(startLocation);
	mWindow->fileCursorY = findLocation.row;
	mWindow->fileCursorX = findLocation.filePos;
	if (findLocation.startCol + findLocation.length >= mWindow->colOffset + mWindow->cols)
	{
		mWindow->colOffset = findLocation.startCol + findLocation.length - mWindow->cols + 1;
	}
}

void Editor::moveCursorToFind(const KeyActions::KeyAction key)
{
	if (mFindLocations.empty()) return;

	switch (key)
	{
	case KeyActions::KeyAction::ArrowLeft:
	case KeyActions::KeyAction::ArrowUp:
		if (mCurrentFindPos == 0) mCurrentFindPos = mFindLocations.size() - 1;
		else --mCurrentFindPos;

		break;

	case KeyActions::KeyAction::ArrowDown:
	case KeyActions::KeyAction::ArrowRight:
	case KeyActions::KeyAction::Enter:
		if (mCurrentFindPos == mFindLocations.size() - 1) mCurrentFindPos = 0;
		else ++mCurrentFindPos;

		break;

	default:
		break;
	}

	mWindow->fileCursorY = mFindLocations.at(mCurrentFindPos).row;
	mWindow->fileCursorX = mFindLocations.at(mCurrentFindPos).filePos;
	if (mFindLocations.at(mCurrentFindPos).startCol + mFindLocations.at(mCurrentFindPos).length >= mWindow->colOffset + mWindow->cols)
	{
		mWindow->colOffset = mFindLocations.at(mCurrentFindPos).startCol + mFindLocations.at(mCurrentFindPos).length - mWindow->cols + 1;
	}
}

void Editor::replaceFindString(const std::string& replaceStr, const bool replaceAll)
{
	if (mFindLocations.empty()) return;

	if (replaceAll)
	{
		for (size_t i = mFindLocations.size() - 1; i > 0; --i)
		{
			const FindAndReplace::FindLocation& current = mFindLocations.at(i);
			FindAndReplace::replace(mWindow->fileRows->at(current.row).line, replaceStr, current);
		}
		FindAndReplace::replace(mWindow->fileRows->at(mFindLocations.front().row).line, replaceStr, mFindLocations.front());
		mFindLocations.clear();
		mCurrentFindPos = 0;
		enableReadMode();
		return;
	}

	const FindAndReplace::FindLocation& current = mFindLocations.at(mCurrentFindPos);
	FindAndReplace::replace(mWindow->fileRows->at(current.row).line, replaceStr, current);

	if (replaceStr.length() != current.length)
	{
		const int16_t lengthDiff = replaceStr.length() - current.length;
		for (size_t i = mCurrentFindPos + 1; i < mFindLocations.size(); ++i)
		{
			FindAndReplace::FindLocation& locationToUpdate = mFindLocations.at(i);
			if (locationToUpdate.row > current.row) break; //No more to update

			locationToUpdate.filePos += lengthDiff;

			const size_t tabSpaces = getRenderedTabSpaces(mWindow->fileRows->at(locationToUpdate.row), locationToUpdate.filePos);
			locationToUpdate.startCol = locationToUpdate.filePos + tabSpaces;
		}
	}

	mFindLocations.erase(mFindLocations.begin() + mCurrentFindPos);
	if (mFindLocations.empty()) mCurrentFindPos = 0;
	else if (mCurrentFindPos == mFindLocations.size()) --mCurrentFindPos;

	moveCursorToFind(KeyActions::KeyAction::None);
}