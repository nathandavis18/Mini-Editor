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

	const bool hasSyntax()
	{
		return currentSyntax != nullptr;
	}

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

	void initSyntax(const std::string_view fName)
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

	const std::vector<HighlightLocations>& highlightLocations()
	{
		return highlights;
	}

	uint8_t color(HighlightType type)
	{
		return colors[static_cast<uint8_t>(type)];
	}

	void findEndMarker(std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, size_t& row, size_t& posOffset, size_t& findPos, size_t startRow, size_t startCol, const std::string_view& strToFind, const HighlightType hlType)
	{
		size_t endPos;

		uint8_t offset = static_cast<uint8_t>(strToFind.length()); //If the end marker is longer than 255 characters, get a new end marker lol

		while (true) //Rather than recursive implementation, just stay in the loop until the end marker is found or EOF is reached
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
					{ //If the end marker is preceded by an escape character, and the escape character is not escaped itself
						size_t newOffset = endPos + 1;
						posOffset += newOffset;
						currentWord = currentWord.substr(newOffset);
						continue;
					}
				}
			}
			break;

		}
		size_t endCol = posOffset + endPos + strToFind.length(); //The endCol is 1 character after the end marker
		highlights.emplace_back(hlType, startRow, startCol, row, endCol, true, true);
		currentWord = currentWord.substr(endPos + strToFind.length());
		posOffset += endPos + strToFind.length();
	}

	bool highlightCommentCheck(std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, FileHandler::Row* row, size_t findPos, size_t& posOffset, size_t& i)
	{
		bool gotoNextRow = false;

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
			gotoNextRow = true;
		}
		else
		{
			//Go to the next word
			posOffset += findPos + 1;
			currentWord = currentWord.substr(findPos + 1);
		}
		return gotoNextRow;
	}

	std::tuple<int64_t, int64_t> removeOffScreenHighlights(size_t rowOffset, size_t rows, size_t fileCursorY)
	{
		size_t rowToStart = std::numeric_limits<size_t>::max();
		size_t startColOffset = std::numeric_limits<size_t>::max();

	startover:
		for (size_t i = 0; i < highlights.size(); ++i) //First pass gets rid of all unnecessary syntax highlights (all the off-screen ones)
		{
			if (highlights[i].highlightType == HighlightType::MultilineComment || highlights[i].highlightType == HighlightType::String)
			{
				if (highlights[i].startRow < rowOffset && highlights[i].endRow < rowOffset) //Don't erase this or we will lose the starting point
				{
					highlights[i].drawColor = false; //Don't want to actually set the render color for this since it is offscreen
					continue;
				}
			}

			if (highlights[i].startRow >= rowOffset)
			{
				goto eraseHighlight;
			}
			else if (highlights[i].startRow < rowOffset && !(highlights[i].highlightType == HighlightType::MultilineComment || highlights[i].highlightType == HighlightType::String))
			{ //Can't remove multiline comments and strings just yet, their position may need to be saved
				goto eraseHighlight;
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
					goto eraseHighlight;
				}
			}
			else if (highlights[i].startRow > rowOffset + rows)
			{
				highlights.erase(highlights.begin() + i, highlights.end());
			}
			else if (highlights[i].endRow < rowOffset)
			{
				goto eraseHighlight;
			}
			else if (highlights[i].startRow == fileCursorY)
			{
				goto eraseHighlight;
			}
			else if (highlights[i].endRow == fileCursorY && highlights[i].endFound)
			{
				if (highlights[i].highlightType == HighlightType::String || highlights[i].highlightType == HighlightType::MultilineComment)
				{
					if (highlights[i].startRow < rowOffset && rowToStart == std::numeric_limits<size_t>::max())
					{
						rowToStart = highlights[i].startRow;
						startColOffset = highlights[i].startCol;
					}
				}
				goto eraseHighlight;
			}
			else if (!highlights[i].endFound)
			{
				if (highlights[i].startRow < rowOffset && rowToStart == std::numeric_limits<size_t>::max())
				{
					rowToStart = highlights[i].startRow;
					startColOffset = highlights[i].startCol;
				}
				goto eraseHighlight;
			}

		eraseHighlight:
			highlights.erase(highlights.begin() + i);

			if (i == 0) goto startover;
			--i;
		}

		return std::tuple<int64_t, int64_t>(rowToStart, startColOffset);
	}

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