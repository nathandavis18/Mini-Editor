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
* @file Editor.hpp
* @brief Provides the interface for the Editor
* 
* This file's purpose is to separate the implementation away from other files.
* They don't need to know how the editor works, just need to call specific functions
*/
#pragma once

#include "SyntaxHighlight/SyntaxHighlight.hpp"
#include "KeyActions/KeyActions.hh"
#include "File/File.hpp"
#include "Console/ConsoleInterface.hpp"
#include "FindAndReplace/FindAndReplace.hpp"
#include "Renderer/Renderer.hpp"

#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <deque>
#include <mutex>
#include <cstdint>

class Editor
{
public:
	/// <summary>
	/// A list of modes the editor can be in
	/// </summary>
	enum class Mode
	{
		CommandMode,
		EditMode,
		FindInputMode,
		FindMode,
		ReplaceInputMode,
		ReplaceMode,
		ReadMode,
		ExitMode,
		None
	};

public:
	/// <summary>
	/// Used when another file needs to know what mode the editor is in
	/// </summary>
	/// <returns> A readonly value for the current Mode </returns>
	const Mode mode() const;

	/// <summary>
	/// Initializes the editor. Should only be called on program start.
	/// </summary>
	Editor(SyntaxHighlight syntax, FileHandler fileHandler, std::unique_ptr<IConsole> console, Renderer r = Renderer());

	/// <summary>
	/// When you want to display everything to the user, call this function
	/// Normally the screen only redraws the main text when it has changed. But you can force it to update if needed
	/// </summary>
	/// <param name="forceRedrawScreen"> Forces the main text portion to be redrawn if true </param>
	void refreshScreen(bool forceRedrawScreen = false);

	/// <summary>
	/// When a movement key is pressed, move the cursor.
	/// Keys include:
	///									CTRL-PAGE UP/DOWN
	///		ARROW LEFT/RIGHT/UP/DOWN	CTRL-ARROW LEFT/RIGHT/UP/DOWN
	///		HOME/END					CTRL HOME/END
	/// </summary>
	/// <param name="key"></param>
	void moveCursor(const KeyActions::KeyAction key);

	/// <summary>
	/// When a key that offsets the viewport is pressed, update the row offset
	/// Keys include:
	///		PAGE UP/DOWN
	///		CTRL-ARROW UP/DOWN
	/// </summary>
	/// <param name="key"></param>
	void shiftRowOffset(const KeyActions::KeyAction key);

	/// <summary>
	/// When Enter/Return is pressed, add a new row to the file
	/// Moves contents beyond the cursor onto the new row
	/// </summary>
	void addRow();

	/// <summary>
	/// When a key that deletes a character/characters gets pressed
	/// Keys include:
	///		BACKSPACE	CTRL-BACKSPACE
	///		DELETE		CTRL-DELETE
	/// </summary>
	/// <param name="key"></param>
	void deleteChar(const KeyActions::KeyAction key);

	/// <summary>
	/// When a character key is pressed (a non-action key), insert the character at the current position
	/// Should only be called while in EDIT mode
	/// </summary>
	/// <param name="c"></param>
	void insertChar(const unsigned char c);

	/// <summary>
	/// When CTRL-Z is pressed, undo the change.
	/// First adds the current state of the editor to the Redo Change
	/// Then Un-does the most recent change
	/// </summary>
	void undoChange();

	/// <summary>
	/// When CTRL-Y is pressed, redo the change.
	/// First adds the current state of the editor to Undo Change
	/// Then re-does the most recent undo
	/// </summary>
	void redoChange();

	/// <summary>
	/// Checks if the file has been updated since last save
	/// Should be used when trying to exit the program
	/// </summary>
	/// <returns></returns>
	const bool isDirty() const;

	/// <summary>
	/// Sends the current changes to be written to the file.
	/// Called when either save or save/quit command is used
	/// </summary>
	void save();

	/// <summary>
	/// When command mode is entered (pressing ':' while in read mode), take necessary steps
	/// Steps include:
	///		Disable Raw Mode
	///		Move Cursor to status input row
	///		Set current mode to COMMAND mode
	/// </summary>
	void enableCommandMode();

	/// <summary>
	/// Enables FindInputMode mode, which is used when the user is inputting a find string
	/// </summary>
	void enableFindInputMode();

	/// <summary>
	/// Enables Find mode, which is used to cycle through the find locations, and enter replace mode
	/// </summary>
	void enableFindMode();

	/// <summary>
	/// Enables ReplaceInputMode, which is used when the user is inputting a replace string
	/// </summary>
	void enableReplaceInputMode();

	/// <summary>
	/// Enables Replace mode, which is used when the user is cycling through find locations to replace specific ones (or optionally all of them)
	/// </summary>
	void enableReplaceMode();

	/// <summary>
	/// When changing from read mode to edit mode (when 'i' is pressed while in read mode)
	/// Steps include:
	///		Add new row to file if file is empty
	///		Set mode to EDIT Mode
	///		Enable Raw Input Mode
	/// </summary>
	void enableEditMode();

	/// <summary>
	/// When going from command mode or edit mode to Read mode (when ESC is pressed, or after finishing a command)
	/// Steps include:
	///		Set mode to READ Mode
	///		Enable Raw Input Mode
	/// </summary>
	void enableReadMode();

	/// <summary>
	/// When a quit command is used ('q' when file has been saved, or 'q!' to force quit)
	/// Steps include:
	///		Set mode to EXIT Mode
	///		Disable Raw Input Mode
	/// </summary>
	void enableExitMode();

	/// <summary>
	/// When the window size has changed, or when first initializing the editor, make sure mWindow->rows and cols are up to date
	/// </summary>
	void updateWindowSize();

	/// <summary>
	/// When in command mode, make sure the command that the user has typed doesn't get erased when the screen size changes
	/// </summary>
	/// <param name="command"></param>
	void updateCommandBuffer(const std::string& command);

	/// <summary>
	/// Gets the locations of the strings that match strToFind
	/// </summary>
	/// <param name="strToFind"></param>
	void findString(const std::string& strToFind);

	/// <summary>
	/// Moves the cursor to the next/previous find location depending on key pressed
	/// </summary>
	/// <param name="key"></param>
	void moveCursorToFind(const KeyActions::KeyAction key);

	/// <summary>
	/// Replaces the current find location with the given replaceStr.
	/// If 'a' is pressed, replaces all of the find locations
	/// </summary>
	/// <param name="replaceStr"></param>
	/// <param name="replaceAll"></param>
	void replaceFindString(const std::string& replaceStr, const bool replaceAll = false);

private:
	/// <summary>
	/// The structure for how the window stores information and tracks current position within the file
	/// </summary>
	struct Window
	{
		Window(FileHandler& file);
		size_t fileCursorX, fileCursorY;
		size_t renderedCursorX, renderedCursorY;
		size_t savedRenderedCursorXPos; 
		bool updateSavedPos = true;
		size_t colNumberToDisplay;
		size_t rowOffset, colOffset;
		int rows, cols;

		std::vector<FileHandler::Row>* fileRows;

		bool dirty;
	};

	/// <summary>
	/// The structure for saving necessary information to the undo/redo stacks
	/// </summary>
	struct ChangeHistory
	{
		enum class ChangeType
		{
			CharInserted,
			CharDeleted,
			RowInserted,
			RowDeleted,
			None
		} changeType;

		ChangeHistory(ChangeType change, const Window& window);

		std::string changeMade;
		size_t fileCursorY, fileCursorX, rowOffset, colOffset;
		size_t rowChanged = 0, colChanged = 0;
		size_t prevLineLength = 0;
	};
	
	/// <summary>
	/// Steps that need to be taken before refreshScreen() does its thing.
	/// This sets the rendered string up for each line, including replacing tabs with spaces, and sets the highlight positions.
	/// </summary>
	void prepForRender();

	/// <summary>
	/// Sets the rendered lines that are currently on screen and replaces the tabs with spaces
	/// </summary>
	void setRenderedLine(const size_t startRow, const size_t endRow);

	/// <summary>
	/// Preps the rendered line to be rendered by making sure the line length < console width
	/// Also checks the position of the colOffset compared to the rendered line to make sure it should be rendered at all
	/// </summary>
	void setRenderedLineLength();

	/// <summary>
	/// Gets all the necessary information for the status buffer and sends it to the renderer
	/// </summary>
	void prepStatusForRender();

	/// <summary>
	/// When a move cursor left/right operation is done, handles the common situations between them and returns a specific action code
	/// Action codes include:
	///		cursorCantMove (-1) = When the cursor can't move (i.e. at the end of file and trying to move right)
	///		cursorMovedNewLine (0) = When the cursor was at the end of a line and moved to next line
	///		cursorMoveNormal (1) = When the cursor should just move normally (i.e. 1 character or word over depending on pressed key)
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	int8_t moveCursorLeftRight(const KeyActions::KeyAction key);

	/// <summary>
	/// Called when the cursor is at the start of the row and Backspace is pressed, or end of the row and Delete is pressed
	/// </summary>
	/// <param name="fileCursor"></param>
	void deleteRow(const size_t fileCursor, const size_t rowNumToAppend);

	/// <summary>
	/// Clears all the possible Redo's from the file history queue and sets Redo Count to 0
	/// </summary>
	void clearRedoHistory();

	/// <summary>
	/// Called when a change is being made and we need to save the current state to be able to undo
	/// </summary>
	void addUndoHistory(ChangeHistory::ChangeType change, const int16_t offset = 0);

	/// <summary>
	/// Reverses the ChangeType of the current File History when moving it from being undo history to redo history and vice versa
	/// </summary>
	/// <param name="current"></param>
	/// <returns></returns>
	ChangeHistory::ChangeType reverseChangeType(ChangeHistory::ChangeType current);

	/// <summary>
	/// Called when moving a redo back to the undo queue
	/// Just reverses the change type
	/// </summary>
	void addUndoHistory(ChangeHistory history);

	/// <summary>
	/// Called when an undo happens and we need to save the current state to be able to redo
	/// </summary>
	void addRedoHistory(ChangeHistory history);

	/// <summary>
	/// Called when moving the cursor up/down, retaining the cursor's horizontal position
	/// Accounts for tabs and puts cursor on the closest available spot as it was before
	/// </summary>
	void setCursorLinePosition();

	/// <summary>
	/// Fixes the rendered cursor's x position accounting for tabs, as well as the y position depending on how far way the cursor is from the row offset
	/// </summary>
	/// <param name=""></param>
	void fixRenderedCursorPosition(const FileHandler::Row&);

	/// <summary>
	/// Replaces the tabs in the rendered string with spaces, to the nearest multiple of 8.
	/// Called on each re-render when the rendered line length is > 0.
	/// </summary>
	/// <param name=""></param>
	void replaceRenderedStringTabs(std::string&);

	/// <summary>
	/// Makes sure the rendered cursor is also accounting for tab spacing so the rendered cursor is always aligned with what is actually rendered
	/// </summary>
	/// <param name=""></param>
	/// <returns> The amount of spaces the rendered cursor needs to move </returns>
	const size_t getRenderedTabSpaces(const FileHandler::Row&, size_t endPos) const;

	/// <summary>
	/// Creates the escape code sequence insert adjusments needed when find string highlights have been added
	/// </summary>
	/// <param name="adjustmentsMade"></param>
	/// <param name="findLocation"></param>
	/// <param name="findColorLength"></param>
	/// <returns></returns>
	size_t adjustSyntaxHighlightLocations(const size_t adjustmentsMade, const FindAndReplace::FindLocation& findLocation, const size_t findColorLength);

	/// <summary>
	/// Adds the find location color escape codes to the rendered string
	/// </summary>
	/// <param name="rowOffset"></param>
	/// <param name="colOffset"></param>
	void addFindLocationColor(const size_t rowOffset, const size_t colOffset);

	/// <summary>
	/// Adds the syntax highlight color escape codes to the rendered string
	/// </summary>
	/// <param name="rowOffset"></param>
	/// <param name="colOffset"></param>
	void addSyntaxHighlightColor(const size_t rowOffset, const size_t colOffset);

	/// <summary>
	/// Calls the separate render color functions depending on if certain checks pass
	/// </summary>
	void updateRenderedColor();

	/// <summary>
	/// Finds highlight keywords and adds them to the token system to be drawn
	/// </summary>
	void setHighlightLocations(const size_t rowToStart, size_t colToStart);

private:
	std::string mCommandBuffer;
	std::string mNormalColorMode;

	std::unique_ptr<Window> mWindow;
	std::unique_ptr<IConsole> mConsole;
	FileHandler mFile;
	SyntaxHighlight mSyntax;
	Renderer mRenderer;

	Mode mMode = Mode::ReadMode; //Default mode is Read Mode.

	std::vector<FindAndReplace::FindLocation> mFindLocations;
	size_t mCurrentFindPos = 0;

	std::deque<ChangeHistory> mFileHistory; //Double ended queue - Front for undo history, back for redo history
	size_t mRedoCounter = 0; //Tracking how many redos we can do
	
	std::mutex mMutex;

	//Some constants to give specific values an identifying name
	inline static const std::string_view separators = " \"',.()+-/*=~%;:[]{}<>";
	inline static const std::string normalBackgroundColor = "\x1b[48;5;0m";
	inline static constexpr uint8_t tabSpacing = 8;
	inline static constexpr uint8_t maxSpacesForTab = 7;
	inline static constexpr uint8_t statusMessageRows = 2;

	//Return codes from moveCursorLeftRight()
	inline static constexpr int8_t cursorCantMove = -1;
	inline static constexpr int8_t cursorMovedNewLine = 0;
	inline static constexpr int8_t cursorMoveNormal = 1;

#ifdef TESTING
public:
	const Window getWindowForTesting() const { return *mWindow; } //Need some way to access the file rows and other information when testing
#endif
};