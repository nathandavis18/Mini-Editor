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

#include "SyntaxHighlight.hpp"
#include <array>

namespace SyntaxHighlight
{
	// Color IDs correspond to the IDs found at this link: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#:~:text=Where%20%7BID%7D%20should%20be%20replaced%20with%20the%20color%20index%20from%200%20to%20255%20of%20the%20following%20color%20table%3A
	constexpr uint8_t white = 255;
	constexpr uint8_t lightGreen = 40;
	constexpr uint8_t darkGreen = 28;
	constexpr uint8_t red = 196;
	constexpr uint8_t pinkishPurple = 177;
	constexpr uint8_t purple = 105;
	constexpr uint8_t orange = 215;
	constexpr uint8_t blue = 6;

	std::array<uint8_t, static_cast<uint8_t>(HighlightType::EnumCount)> colors;
	std::vector<EditorSyntax> syntaxContents;
	std::vector<HighlightLocations> highlights;
	EditorSyntax* currentSyntax = nullptr;

	/// <summary>
	/// Returns a pointer (or nullptr) to the current syntax being used.
	/// </summary>
	/// <returns> nullptr if no syntax, or a pointer to the correct syntax </returns>
	const EditorSyntax* syntax()
	{
		return currentSyntax;
	}

	/// <summary>
	/// Setting the color values for each type
	/// </summary>
	void setColors()
	{
		colors[static_cast<uint8_t>(HighlightType::Normal)] = white;
		colors[static_cast<uint8_t>(HighlightType::Comment)] = lightGreen;
		colors[static_cast<uint8_t>(HighlightType::MultilineComment)] = darkGreen;
		colors[static_cast<uint8_t>(HighlightType::KeywordBuiltInType)] = red;
		colors[static_cast<uint8_t>(HighlightType::KeywordControl)] = pinkishPurple;
		colors[static_cast<uint8_t>(HighlightType::KeywordOther)] = purple;
		colors[static_cast<uint8_t>(HighlightType::String)] = orange;
		colors[static_cast<uint8_t>(HighlightType::Number)] = blue;
	}

	/// <summary>
	/// Initializes the syntax index and vector based on the current filetype
	/// </summary>
	/// <param name="fName"></param>
	void initSyntax(const std::string_view& fName)
	{
		syntaxContents.emplace_back(cppFiletypes, cppBuiltInTypes, cppControlKeywords, cppOtherKeywords, "//", "/*", "*/", '\\');

		std::string_view extension;
		size_t extensionIndex;
		if ((extensionIndex = fName.find('.')) != std::string::npos)
		{
			extension = fName.substr(extensionIndex);
		}
		else
		{
			return; //There isn't a syntax, so we can't provide syntax highlighting. 
		}

		for (uint8_t i = 0; i < syntaxContents.size(); ++i)
		{
			if (syntaxContents[i].filematch.contains(extension)) 
			{
				setColors();
				currentSyntax = &syntaxContents[i];
				return;
			}
		}
	}

	std::vector<HighlightLocations>& highlightLocations()
	{
		return highlights;
	}

	/// <summary>
	/// Returns the color ID of a given highlight type
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	uint8_t color(HighlightType type)
	{
		return colors[static_cast<uint8_t>(type)];
	}

	/// <summary>
	/// Tries to find the end marker, denoted as strToFind
	/// Either runs until the max row count is reached or until it is found
	/// </summary>
	/// <param name="fileRows"> The rows within the current file </param>
	/// <param name="currentWord"> The 'word' we are checking to find the end marker </param>
	/// <param name="row"> The current row. Must be reference, since the setHighlight funciton depends on this </param>
	/// <param name="posOffset"> The offset within the current word, so we don't re-check the same stuff </param>
	/// <param name="findPos"> Where the starting marker was found </param>
	/// <param name="startRow"> What row we started on, since this may be multiple rows in length </param>
	/// <param name="startCol"></param>
	/// <param name="strToFind"> What end marker we are trying to find </param>
	/// <param name="hlType"> The type of highlight </param>
	void findEndMarker(std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, size_t& row, size_t& posOffset, size_t& findPos, size_t startRow, size_t startCol, const std::string_view& strToFind, const HighlightType hlType)
	{
		size_t endPos;

		uint8_t offset = static_cast<uint8_t>(strToFind.length()); //If the end marker is longer than 255 characters, get a new end marker lol

		while (true)
		{
			while ((endPos = currentWord.find(strToFind, offset)) == std::string::npos)
			{
				findPos = 0;
				posOffset = 0;
				offset = 0;
				++row;
				if (row >= fileRows.size())
				{
					highlights.emplace_back(hlType, startRow, startCol, row - 1, fileRows.at(row - 1).renderedLine.length(), false, true);
					currentWord = std::string_view();
					return;
				}
				currentWord = fileRows.at(row).renderedLine;
			}
			if (endPos >= 1) //If there is a chance for there to be an escape char
			{
				if (currentWord[endPos - 1] == currentSyntax->escapeChar)
				{
					if (!(endPos > 1 && currentWord.substr(endPos - 2, 2) == std::string() + currentSyntax->escapeChar + currentSyntax->escapeChar))
					{
						size_t newOffset = endPos + 1;
						posOffset += newOffset;
						currentWord = currentWord.substr(newOffset);
						continue;
					}
				}
			}
			break;

		}
		size_t endCol = posOffset + endPos + strToFind.length();
		highlights.emplace_back(hlType, startRow, startCol, row, endCol, true, true);
		currentWord = currentWord.substr(endPos + strToFind.length());
		posOffset += endPos + strToFind.length();
	}

	/// <summary>
	/// Adds the highlight positions for comments and strings
	/// </summary>
	/// <param name="fileRows"> The rows within the current file </param>
	/// <param name="currentWord"></param>
	/// <param name="row"></param>
	/// <param name="findPos"></param>
	/// <param name="posOffset"></param>
	/// <param name="i"></param>
	/// <returns>True if we need to go to the next row, otherwise false</returns>
	bool highlightCommentCheck(std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, FileHandler::Row* row, size_t findPos, size_t& posOffset, size_t& i)
	{
		const uint8_t singlelineCommentLength = static_cast<uint8_t>(currentSyntax->singlelineComment.length()); //If the comment character is longer than 255 characters, just don't. Find a new character
		const uint8_t multilineCommentLength = static_cast<uint8_t>(currentSyntax->multilineCommentStart.length()); //If the comment character is longer than 255 characters, just don't. Find a new character
		if (currentWord[findPos] == '"' || currentWord[findPos] == '\'') //String highlights are open until the next string marker of the same type is found
		{
			posOffset += findPos;
			size_t startCol = posOffset;
			size_t startRow = i;
			currentWord = currentWord.substr(findPos);
			findEndMarker(fileRows, currentWord, i, posOffset, findPos, startRow, startCol, std::string() + currentWord[0], HighlightType::String);
		}
		else if (findPos + multilineCommentLength - 1 < currentWord.length() //Multiline comments stay open until the closing marker is found
			&& currentWord.substr(findPos, multilineCommentLength) == currentSyntax->multilineCommentStart)
		{
			posOffset += findPos;
			size_t startCol = posOffset;
			size_t startRow = i;
			currentWord = currentWord.substr(findPos);
			findEndMarker(fileRows, currentWord, i, posOffset, findPos, startRow, startCol, currentSyntax->multilineCommentEnd, HighlightType::MultilineComment);
		}
		else if (findPos + singlelineCommentLength - 1 < currentWord.length() //Singleline comments take the rest of the row
			&& currentWord.substr(findPos, singlelineCommentLength) == currentSyntax->singlelineComment)
		{
			highlights.emplace_back(HighlightType::Comment, i, findPos + posOffset, i, row->renderedLine.length());
			return true;
		}
		else
		{
			//Go to the next word
			posOffset += findPos + 1;
			currentWord = currentWord.substr(findPos + 1);
		}
		return false;
	}

	/// <summary>
	/// Removes off-screen highlights from the highlight vector so there are no unnecessary checks
	/// </summary>
	/// <param name="rowOffset"> The current row offset </param>
	/// <param name="rows"> The number of rows that can be displayed </param>
	/// <param name="fileCursorY"> The current cursor position </param>
	/// <returns>A tuple of the row to start on and the column offset of that row</returns>
	std::tuple<int64_t, int64_t> removeOffScreenHighlights(size_t rowOffset, size_t rows, size_t fileCursorY)
	{
		int64_t rowToStart = -1;
		int64_t startColOffset = -1;

		for (int64_t i = 0; i < highlights.size(); ++i) //First pass gets rid of all unnecessary syntax highlights (all the off-screen ones)
		{
			if (highlights[i].highlightType == HighlightType::MultilineComment || highlights[i].highlightType == HighlightType::String)
			{
				if (highlights[i].startRow < rowOffset && highlights[i].endRow < rowOffset) //Don't erase this or we will lose the starting point
				{
					highlights[i].drawColor = false; //Don't want to actually set the render color for this since it is offscreen
					continue;
				}
			}
			if (highlights[i].startRow < rowOffset && !(highlights[i].highlightType == HighlightType::MultilineComment || highlights[i].highlightType == HighlightType::String))
			{
				highlights.erase(highlights.begin() + i);
				--i;
			}
			else if (highlights[i].startRow > rowOffset + rows)
			{
				highlights.erase(highlights.begin() + i, highlights.end());
			}
			else if (highlights[i].endRow < rowOffset)
			{
				highlights.erase(highlights.begin() + i);
				--i;
			}
			else if (highlights[i].startRow == fileCursorY)
			{
				highlights.erase(highlights.begin() + i);
				--i;
			}
			else if (highlights[i].endRow == fileCursorY && highlights[i].endFound)
			{
				if (highlights[i].highlightType == HighlightType::String || highlights[i].highlightType == HighlightType::MultilineComment)
				{
					if (highlights[i].startRow < rowOffset && rowToStart == -1)
					{
						rowToStart = highlights[i].startRow;
						startColOffset = highlights[i].startCol;
					}
				}
				highlights.erase(highlights.begin() + i);
				--i;
			}
			else if (!highlights[i].endFound)
			{
				if (highlights[i].startRow < rowOffset && rowToStart == -1)
				{
					rowToStart = highlights[i].startRow;
					startColOffset = highlights[i].startCol;
				}
				highlights.erase(highlights.begin() + i);
				--i;
			}
		}

		//2nd pass clears the ones that need to be updated. Essentially, all that should be left is multiline comments and quotes.
		for (int64_t i = 0; i < highlights.size(); ++i)
		{
			if (highlights[i].startRow >= rowOffset)
			{
				highlights.erase(highlights.begin() + i);
				--i;
			}
			else if (highlights[i].endRow >= rowOffset && highlights[i].endRow <= rowOffset + rows)
			{
				if (highlights[i].highlightType == HighlightType::String || highlights[i].highlightType == HighlightType::MultilineComment)
				{
					if (highlights[i].startRow < rowOffset)
					{
						rowToStart = highlights[i].startRow;
						startColOffset = highlights[i].startCol;
					}
					highlights.erase(highlights.begin() + i);
					--i;
				}
			}
		}

		return std::tuple<int64_t, int64_t>(rowToStart, startColOffset);
	}

	/// <summary>
	/// Adds the highlight sections for keywords and numbers, since they don't need any special treatment
	/// </summary>
	/// <param name="currentWord"></param>
	/// <param name="i"></param>
	/// <param name="posOffset"></param>
	void highlightKeywordNumberCheck(std::string_view& currentWord, size_t i, size_t posOffset)
	{
		if (currentWord.find_first_not_of("0123456789") == std::string::npos)
		{
			highlights.emplace_back(HighlightType::Number, i, posOffset, i, posOffset + currentWord.length());
			return;
		}
		else
		{
			if (currentSyntax->builtInTypeKeywords.contains(std::string(currentWord)))
			{
				highlights.emplace_back(HighlightType::KeywordBuiltInType, i, posOffset, i, posOffset + currentWord.length());
				return;
			}
			else if (currentSyntax->loopKeywords.contains(std::string(currentWord)))
			{
				highlights.emplace_back(HighlightType::KeywordControl, i, posOffset, i, posOffset + currentWord.length());
				return;
			}
			else if (currentSyntax->otherKeywords.contains(std::string(currentWord)))
			{
				highlights.emplace_back(HighlightType::KeywordOther, i, posOffset, i, posOffset + currentWord.length());
				return;
			}
		}
	}
}