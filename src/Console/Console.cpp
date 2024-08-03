#include "Console.hpp"
#include "../KeyActions/KeyActions.hh"
#include <iostream>
#include <fstream>
#include <format>
#define NuttyVersion "0.1a"

namespace Console
{
	#ifdef _WIN32 //Windows Console stuff	
	Window window;
	size_t fileNumRows = 0;
	Window::Window() : cursorX(0), cursorY(0), rowOffset(0), colOffset(0), dirty(false), rawModeEnabled(false), statusMessage("Test Status Message Length go BRR"), fileRows(FileHandler::rows())
	{
		CONSOLE_SCREEN_BUFFER_INFO screenInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_ERROR_HANDLE), &screenInfo);

		constexpr uint8_t statusMessageRows = 2;
		rows = screenInfo.srWindow.Bottom - screenInfo.srWindow.Top + 1 - statusMessageRows;
		cols = screenInfo.srWindow.Right - screenInfo.srWindow.Left + 1;
	}

	DWORD defaultMode;
	void initConsole()
	{
		if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &defaultMode))
		{
			std::cerr << "Error retrieving current console mode";
			exit(EXIT_FAILURE);
		}
		if (!(window.rawModeEnabled = enableRawInput()))
		{
			std::cerr << "Error enabling raw input mode";
			exit(EXIT_FAILURE);
		}
		fileNumRows = window.fileRows.size();
		if (window.fileRows.size() < window.rows)
		{
			window.fileRows.resize(window.rows);
		}
	}
	bool enableRawInput()
	{
		DWORD rawMode = ENABLE_EXTENDED_FLAGS | (defaultMode & ~ENABLE_LINE_INPUT & ~ENABLE_PROCESSED_INPUT 
					 & ~ENABLE_ECHO_INPUT & ~ENABLE_PROCESSED_OUTPUT & ~ENABLE_WRAP_AT_EOL_OUTPUT); //Disabling input modes to enable raw mode

		atexit(Console::disableRawInput);
		window.rawModeEnabled = true;
		return SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), rawMode);
	}
	void disableRawInput()
	{
		if (window.rawModeEnabled)
		{
			SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), defaultMode);
			window.rawModeEnabled = false;
		}
	}
#elif __linux__ || __APPLE__
	//Apple/Linux raw mode console stuff
#endif //OS Terminal Raw Mode

	void refreshScreen(const std::string_view& mode)
	{
		FileHandler::Row* row;
		std::string aBuffer;

		aBuffer.append("\x1b[?251");
		aBuffer.append("\x1b[H");
		for (size_t y = 0; y < window.rows; ++y)
		{
			size_t fileRow = window.rowOffset + y;
			if (fileRow >= fileNumRows)
			{
				if (fileNumRows == 0 && y == window.rows / 3)
				{
					std::string welcome = std::format("Nutty Editor -- version {}\x1b[0K\r\n", NuttyVersion);
					size_t welcomeLength = welcome.length();
					size_t padding = (window.cols - welcomeLength) / 2;
					if (padding > 0)
					{
						aBuffer.append("~");
						--padding;
					}
					while (padding > 0)
					{
						aBuffer.append(" ");
						--padding;
					}
					aBuffer.append(welcome);
				}
				else
				{
					aBuffer.append("~\x1b[0K\r\n");
				}
				continue;
			}
			row = &window.fileRows[fileRow];

			size_t length = row->line.length() - window.colOffset;
			if (length > 0)
			{
				if (length > window.cols) length = window.cols;
				if (row->line.length() > window.colOffset)
				{
					std::string s = row->line.substr(window.colOffset);
					for (size_t j = 0; j < length; ++j)
					{
						aBuffer.append("\x1b[39m");
						aBuffer += s[j];
					}
				}
			}
			aBuffer.append("\x1b[39m");
			aBuffer.append("\x1b[0K");
			aBuffer.append("\r\n");
		}

		aBuffer.append("\x1b[0K");
		aBuffer.append("\x1b[7m");

		std::string status, rStatus;
		status = std::format("{} - {} lines {}", FileHandler::fileName(), fileNumRows, window.dirty ? "(modified)" : "");
		rStatus = std::format("row {}/{} col {}", window.rowOffset + window.cursorY, fileNumRows, window.colOffset + window.cursorX);
		size_t statusLength = (status.length() > window.cols) ? window.cols : status.length();
		aBuffer.append(status);

		while (statusLength < (window.cols / 2))
		{
			if ((window.cols / 2) - statusLength == mode.length() / 2)
			{
				aBuffer.append(mode);
				break;
			}
			else
			{
				aBuffer.append(" ");
				++statusLength;
			}
		}
		statusLength += mode.length();
		
		while (statusLength < window.cols)
		{
			if (window.cols - statusLength == rStatus.length())
			{
				aBuffer.append(rStatus);
				break;
			}
			else
			{
				aBuffer.append(" ");
				++statusLength;
			}
		}

		aBuffer.append("\x1b[0m\r\n");
		aBuffer.append("\x1b[0K");


		size_t cx = 1;
		size_t fileRow = window.rowOffset + window.cursorY;
		FileHandler::Row* cursorRow = (fileRow >= fileNumRows) ? nullptr : &window.fileRows[fileRow];
		if (cursorRow)
		{
			for (size_t j = window.colOffset; j < (window.cursorX + window.colOffset); ++j, ++cx)
			{
				if (j < cursorRow->renderedSize && cursorRow->line[j] == static_cast<char>(KeyActions::KeyAction::Tab))
				{
					cx += 3 - (cx % 4);
				}
			}
		}
		std::string cursorPosition = std::format("\x1b[{};{}H", window.cursorY + 1, cx);
		aBuffer.append(cursorPosition);
		aBuffer.append("\x1b[?25h");
		std::cout << aBuffer;
	}

	void moveCursor(const int key)
	{
		size_t fileRow = window.rowOffset + window.cursorY;
		size_t fileCol = window.colOffset + window.cursorX;
		FileHandler::Row* row = (fileRow >= window.rows) ? nullptr : &window.fileRows[fileRow];

		switch (key)
		{
		case static_cast<int>(KeyActions::KeyAction::ArrowLeft):
			if (window.cursorX == 0)
			{
				if (window.colOffset > 0)
				{
					--window.colOffset;
				}
				else
				{
					if (fileRow > 0)
					{
						--window.cursorY;
						window.cursorX = window.fileRows[fileRow - 1].line.length();
						if (window.cursorX > window.cols - 1)
						{
							window.colOffset = window.cursorX - window.cols + 1;
							window.cursorX = window.cols - 1; 
						}
					}
				}
			}
			else
			{
				--window.cursorX;
			}
			break;
		case static_cast<int>(KeyActions::KeyAction::ArrowRight):
			if (row && fileCol < row->line.length())
			{
				if (window.cursorX == window.cols - 1)
				{
					++window.colOffset;
				}
				else
				{
					++window.cursorX;
				}
			}
			else if (row && fileCol == row->line.length())
			{
				if (window.cursorY + window.rowOffset == fileNumRows - 1) break;

				window.cursorX = 0;
				window.colOffset = 0;
				if (window.cursorY == window.rows - 1)
				{
					++window.rowOffset;
				}
				else
				{
					++window.cursorY;
				}
			}
			break;
		case static_cast<int>(KeyActions::KeyAction::ArrowUp):
			if (window.cursorY == 0)
			{
				if (window.rowOffset > 0)
				{
					--window.rowOffset;
				}
			}
			else
			{
				--window.cursorY;
			}
			break;
		case static_cast<int>(KeyActions::KeyAction::ArrowDown):
			if (fileRow <= window.rows)
			{
				if (window.cursorY + window.rowOffset == fileNumRows) break;
				if (window.cursorY == window.rows - 1)
				{
					++window.rowOffset;
				}
				else
				{
					++window.cursorY;
				}
			}
			break;
		}

		fileRow = window.rowOffset + window.cursorY;
		fileCol = window.colOffset + window.cursorX;

		row = (fileRow >= window.rows) ? nullptr : &window.fileRows[fileRow];
		size_t rowLength = 0;
		if (row)
		{
			rowLength = row->line.length();
		}

		if (fileCol > rowLength)
		{
			window.cursorX -= fileCol - rowLength;
			if (window.cursorX < 0)
			{
				window.colOffset += window.cursorX;
				window.cursorX = 0;
			}
		}
	}

	void deleteChar(const int key)
	{
		size_t fileRow = window.cursorY + window.rowOffset;
		size_t fileCol = window.cursorX + window.colOffset;

		FileHandler::Row* row = (fileRow >= fileNumRows) ? nullptr : &window.fileRows[fileRow];

		if (!row || (fileCol == 0 && fileRow == 0 && key == static_cast<int>(KeyActions::KeyAction::Backspace))) return;
		if (fileCol == 0 && key == static_cast<int>(KeyActions::KeyAction::Backspace))
		{
			fileCol = window.fileRows[fileRow - 1].line.length();
			window.fileRows[fileRow - 1].line.append(row->line);
			deleteRow(fileRow);
			row = nullptr;

			if (window.cursorY == 0)
			{
				--window.rowOffset;
			}
			else
			{
				--window.cursorY;
			}

			window.cursorX = fileCol;
			if (window.cursorX >= window.cols)
			{
				size_t shiftAmount = window.cols - window.cursorX + 1;
				window.cursorX -= shiftAmount;
				window.colOffset += shiftAmount;
			}
		}
		else if(key == static_cast<int>(KeyActions::KeyAction::Backspace))
		{
			row->line.erase(row->line.begin() + fileCol - 1);
			if (window.cursorX == 0 && window.colOffset > 0)
			{
				--window.colOffset;
			}
			else
			{
				--window.cursorX;
			}
		}
		window.dirty = true;
	}
	void deleteRow(const size_t rowNum)
	{
		if (rowNum > fileNumRows) return;
		window.fileRows.erase(window.fileRows.begin() + rowNum);
		window.dirty = true;
		--fileNumRows;
	}

	void addRow()
	{
		size_t fileCol = window.cursorX + window.colOffset;
		size_t fileRow = window.cursorY + window.rowOffset;

		FileHandler::Row* row = (fileRow >= fileNumRows) ? nullptr : &window.fileRows[fileRow];
		if (!row) return;
		if (fileCol >= row->line.length())
		{
			window.fileRows.insert(window.fileRows.begin() + fileRow + 1, FileHandler::Row());
		}
		else if (fileCol == 0 && window.colOffset == 0)
		{
			window.fileRows.insert(window.fileRows.begin() + fileRow, FileHandler::Row());
		}
		else
		{
			FileHandler::Row newRow;
			newRow.line = row->line.substr(fileCol);
			row->line.erase(row->line.begin() + fileCol, row->line.end());
			window.fileRows.insert(window.fileRows.begin() + fileRow + 1, newRow);
		}
		window.cursorX = 0; window.colOffset = 0;
		if (window.cursorY >= window.rows - 1)
		{
			++window.rowOffset;
		}
		else
		{
			++window.cursorY;
		}
		++fileNumRows;
		window.dirty = true;
	}
	void insertChar(const char c)
	{
		size_t fileCol = window.cursorX + window.colOffset;
		size_t fileRow = window.cursorY + window.rowOffset;

		FileHandler::Row* row = (fileRow >= fileNumRows) ? nullptr : &window.fileRows[fileRow];
		if (!row) return;

		size_t length = row->line.length();
		row->line.insert(row->line.begin() + fileCol, c);
		size_t x = row->line.size();
		if (window.cursorX >= window.cols)
		{
			++window.colOffset;
		}
		else
		{
			++window.cursorX;
		}
		window.dirty = true;
	}

	bool isRawMode()
	{
		return window.rawModeEnabled;
	}

	bool isDirty()
	{
		return window.dirty;
	}

	void save()
	{
		std::string output;
		for (size_t i = 0; i < fileNumRows; ++i)
		{
			if (i == fileNumRows - 1)
			{
				output.append(window.fileRows[i].line);
			}
			else
			{
				output.append(window.fileRows[i].line + "\n");
			}
		}
		FileHandler::saveFile(output);
		window.dirty = false;
	}

	void setCursorCommand()
	{
		window.cursorX = 0; window.cursorY = window.rows + 2;
	}
	void setCursorInsert()
	{
		window.cursorX = 0; window.cursorY = 0;
	}

	void shiftRowOffset(const int key)
	{
		if (key == static_cast<int>(KeyActions::KeyAction::CtrlArrowDown))
		{
			if (window.rowOffset == fileNumRows) return;
			size_t fileCol = window.cursorX + window.colOffset;
			size_t fileRow = window.cursorY + window.rowOffset;
			if (window.rowOffset == fileNumRows - 1)
			{
				if (window.fileRows[fileRow].line.length() > (window.colOffset + window.cols))
				{
					window.colOffset += window.fileRows[fileRow].line.length() - (window.colOffset + window.cols);
					window.cursorX = window.cols;
				}
				else
				{
					window.cursorX = window.fileRows[fileRow].line.length();
				}
				return;
			}

			++window.rowOffset;
			++fileRow;
			if (fileRow > fileNumRows - 1)
			{
				--window.cursorY;
			}
			if (fileCol > window.fileRows[fileRow].line.length())
			{
				window.cursorX = window.fileRows[fileRow].line.length() - window.colOffset;
				return;
			}
		}
		else if (key == static_cast<int>(KeyActions::KeyAction::CtrlArrowUp))
		{
			if (window.rowOffset == 0) return;
			if (window.rowOffset == 1)
			{
				window.cursorX = 0;
				window.colOffset = 0;
				return;
			}

			--window.rowOffset;
			size_t fileCol = window.cursorX + window.colOffset;
			size_t fileRow = window.cursorY + window.rowOffset;
			if (fileCol > window.fileRows[fileRow].line.length())
			{
				window.cursorX = window.fileRows[fileRow].line.length() - window.colOffset;
				return;
			}
		}
	}
}
