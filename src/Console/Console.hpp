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
* @file Console.hpp
* @brief Provides the interface for the Console
* 
* This file's purpose is to separate the implementation away from other files.
* They don't need to know how the console works, just need to call specific functions
*/
#pragma once
#include "SyntaxHighlight/SyntaxHighlight.hpp"
#include "KeyActions/KeyActions.hh"
#include "File/File.hpp"

#include <vector>
#include <string>
#include <memory>
#include <stack>
#include <tuple>

/// <summary>
/// A list of modes the editor can be in
/// </summary>
enum class Mode
{
	CommandMode,
	EditMode,
	FindMode, //Currently unused, working on implementation
	ReadMode,
	ExitMode,
	None
};

class Console
{
public:
	static Mode& mode(Mode = Mode::None);
	static void prepRenderedString();
	static void refreshScreen(bool forceRedrawScreen = false);
	static void moveCursor(const KeyActions::KeyAction key);
	static void shiftRowOffset(const KeyActions::KeyAction key);
	static void addRow();
	static void deleteChar(const KeyActions::KeyAction key);
	static void insertChar(const unsigned char c);
	static void undoChange();
	static void redoChange();
	static bool isRawMode();
	static bool isDirty();
	static void save();
	static void enableCommandMode();
	static void enableEditMode();

	//OS Specific Functions
	static void initConsole(const std::string_view&);
	static bool setWindowSize();
	static bool enableRawInput();
	static void disableRawInput();

private:

	/// <summary>
	/// The structure for how the window stores information and tracks current position within the file
	/// </summary>
	struct Window
	{
		Window();
		size_t fileCursorX, fileCursorY;
		size_t renderedCursorX, renderedCursorY;
		size_t savedRenderedCursorXPos; bool updateSavedPos = true;
		size_t colNumberToDisplay;
		size_t rowOffset, colOffset;
		size_t rows, cols;

		std::vector<FileHandler::Row> fileRows;

		bool dirty;
		bool rawModeEnabled;
	};

	/// <summary>
	/// The structure for saving necessary information to the undo/redo stacks
	/// </summary>
	struct FileHistory
	{
		std::vector<FileHandler::Row> rows;
		size_t fileCursorX, fileCursorY;
		size_t colOffset, rowOffset;
	};

	static void renderStatusAndCursor();
	static void prepRenderedLineForRender();
	static void renderEndOfFile();
	static int8_t moveCursorLeftRight(const KeyActions::KeyAction key);
	static void deleteRow(const size_t rowNum);
	static void addUndoHistory();
	static void addRedoHistory();
	static void setRenderedString();
	static void setCursorLinePosition();
	static void fixRenderedCursorPosition(const FileHandler::Row&);
	static void replaceRenderedStringTabs(std::string&);
	static size_t getRenderedCursorTabSpaces(const FileHandler::Row&);
	static void updateRenderedColor(const size_t rowOffset, const size_t colOffset);
	static void setHighlight();

private:
	inline static std::string mRenderBuffer, mPreviousRenderBuffer; //Implementing double-buffering so the screen doesn't need to always update

	inline static std::unique_ptr<Window> mWindow;
	inline static const std::vector<SyntaxHighlight::HighlightLocations>& mHighlights = SyntaxHighlight::highlightLocations();
	inline static std::stack<FileHistory> mRedoHistory;
	inline static std::stack<FileHistory> mUndoHistory;
	inline static Mode mMode = Mode::ReadMode;


	//Some constants to give specific values an identifying name
	inline static const std::string_view separators = " \"',.()+-/*=~%;:[]{}<>";
	static constexpr uint8_t tabSpacing = 8;
	static constexpr uint8_t maxSpacesForTab = 7;
	static constexpr uint8_t statusMessageRows = 2;

	//Return codes from moveCursorLeftRight()
	static constexpr int8_t cursorCantMove = -1;
	static constexpr int8_t cursorMovedNewLine = 0;
	static constexpr int8_t cursorMoveNormal = 1;
};