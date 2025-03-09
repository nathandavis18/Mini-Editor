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

#include "SyntaxHighlight.hpp"
#include "Utility/GetProgramPath/GetProgramPath.hpp"

#include <limits>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>

using JsonParser::JsonValue;
using JsonParser::JsonObject;
using JsonParser::JsonSet;

SyntaxHighlight::SyntaxHighlight(const std::string_view extension) : mColors{ 0 }
{
	if (extension.length() == 0) return;

	std::filesystem::path configPath = GetProgramPath::getPath() / "config.json";
	std::ifstream file(configPath);
	std::stringstream fContents;
	fContents << file.rdbuf();
	mFileContents = fContents.str();
	std::vector<JsonObject> mp = JsonParser::parseJson(mFileContents);
	setSyntax(mp, std::string(extension));
}

const bool SyntaxHighlight::hasSyntax() const
{
	return mCurrentSyntax != nullptr;
}

void SyntaxHighlight::setSyntax(const std::vector<JsonObject>& mp, const std::string extension)
{
	for (const auto& syntax : mp) //For each top level key
	{
		JsonValue currentSyntax = (*syntax.begin()).second; //Get the value of the top-level key (no restrictions on naming convention, so this is the only way to do it)
		if (currentSyntax.contains("fileExtensions"))
		{
			//If the current syntax contains what file extensions are allowed, and if those file extensions contains the current extension
			if (currentSyntax.get<JsonSet>("fileExtensions").contains(extension))
			{
				setColors(currentSyntax);
				setEditorSyntax(currentSyntax);
				break;
			}
		}
	}
}

void SyntaxHighlight::setEditorSyntax(const JsonValue& syntax)
{
	EditorSyntax s;

	if(syntax.contains("builtInKeywords"))	 s.builtInTypeKeywords = syntax.at("builtInKeywords").get<JsonSet>("keywords");
	if(syntax.contains("controlKeywords"))	 s.controlKeywords = syntax.at("controlKeywords").get<JsonSet>("keywords");
	if(syntax.contains("otherKeywords"))	 s.otherKeywords = syntax.at("otherKeywords").get<JsonSet>("keywords");
	if (syntax.contains("multiLineComment"))
	{
		//If multiLineComment exists, both the start and end identifiers must be present
		if (syntax.at("multiLineComment").contains("start") && syntax.at("multiLineComment").contains("end"))
		{
			s.multilineCommentStart = syntax.at("multiLineComment").get<std::string>("start");
			s.multilineCommentEnd = syntax.at("multiLineComment").get<std::string>("end");
		}
	}
	if(syntax.contains("singleLineComment")) s.singlelineComment = syntax.at("singleLineComment").get<std::string>("identifier");
	if(syntax.contains("escapeChar"))		 s.escapeChar = syntax.get<std::string>("escapeChar")[0];

	mCurrentSyntax = std::make_unique<EditorSyntax>(s);
}

/// <summary>
/// Gets the color for a given key, or sets it to the alt color if it doesn't exist
/// </summary>
/// <param name="key"></param>
/// <param name="altColor"></param>
/// <param name="syntax"></param>
/// <returns></returns>
static const std::string getColor(const std::string& key, const std::string& altColor, const JsonValue& syntax)
{
	return syntax.contains(key) ? syntax.at(key).get<std::string>("color") : altColor;
}

void SyntaxHighlight::setColors(const JsonValue& syntax)
{
	//Default color follows a slightly different pattern
	const std::string color = syntax.contains("defaultColor") ? syntax.get<std::string>("defaultColor") : "white";
	mColors[static_cast<uint8_t>(HighlightType::Normal)] = mColorKeys.at(color);

	mColors[static_cast<uint8_t>(HighlightType::Comment)] = mColorKeys.at(getColor("singleLineComment", "limegreen", syntax));

	mColors[static_cast<uint8_t>(HighlightType::MultilineComment)] = mColorKeys.at(getColor("multiLineComment", "green", syntax));

	mColors[static_cast<uint8_t>(HighlightType::KeywordBuiltInType)] = mColorKeys.at(getColor("builtInKeywords", "red", syntax));

	mColors[static_cast<uint8_t>(HighlightType::KeywordControl)] = mColorKeys.at(getColor("controlKeywords", "magenta", syntax));

	mColors[static_cast<uint8_t>(HighlightType::KeywordOther)] = mColorKeys.at(getColor("otherKeywords", "darkpurple", syntax));
	
	mColors[static_cast<uint8_t>(HighlightType::String)] = mColorKeys.at(getColor("string", "orange", syntax));

	mColors[static_cast<uint8_t>(HighlightType::Number)] = mColorKeys.at(getColor("number", "seablue", syntax));
}

const std::vector<SyntaxHighlight::HighlightLocation>& SyntaxHighlight::highlights() const
{
	return mHighlights;
}

const uint8_t SyntaxHighlight::color(HighlightType type) const
{
	return mColors[static_cast<uint8_t>(type)];
}

void SyntaxHighlight::findEndMarker(const std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, size_t& row, size_t& posOffset, size_t& findPos, size_t startRow, size_t startCol, const std::string_view& strToFind, const HighlightType hlType)
{
	size_t endPos;

	uint8_t offset = static_cast<uint8_t>(strToFind.length()); // If the end marker is longer than 255 characters, get a new end marker lol

	while (true) // Rather than recursive implementation, just stay in the loop until the end marker is found or EOF is reached
	{
		while ((endPos = currentWord.find(strToFind, offset)) == std::string::npos)
		{
			findPos = 0;
			posOffset = 0;
			offset = 0;
			++row;
			if (row >= fileRows.size())
			{
				mHighlights.emplace_back(hlType, startRow, startCol, row - 1, fileRows.at(row - 1).renderedLine.length(), false, true);
				currentWord = std::string_view();
				return;
			}
			currentWord = fileRows.at(row).renderedLine;
		}
		if (hlType == HighlightType::String && endPos >= 1) // If there is a chance for there to be an escape char, and the current highlight is a string
		{
			if (currentWord[endPos - 1] == mCurrentSyntax->escapeChar)
			{
				if (!(endPos >= 2 && currentWord[endPos - 2] == mCurrentSyntax->escapeChar))
				{
					size_t newOffset = endPos + 1;
					posOffset += newOffset;
					currentWord = currentWord.substr(newOffset);
					offset = 0;
					continue;
				}
			}
		}
		break;
	}
	size_t endCol = posOffset + endPos + strToFind.length(); // The endCol is 1 character after the end marker
	mHighlights.emplace_back(hlType, startRow, startCol, row, endCol, true, true);
	currentWord = currentWord.substr(endPos + strToFind.length());
	posOffset += endPos + strToFind.length();
}

bool SyntaxHighlight::highlightCommentCheck(const std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, FileHandler::Row* row, size_t findPos, size_t& posOffset, size_t& i)
{
	bool gotoNextRow = false;

	const uint8_t singlelineCommentLength = static_cast<uint8_t>(mCurrentSyntax->singlelineComment.length());	// If the comment character is longer than 255 characters, just don't. Find a new character
	const uint8_t multilineCommentLength = static_cast<uint8_t>(mCurrentSyntax->multilineCommentStart.length()); // If the comment character is longer than 255 characters, just don't. Find a new character
	if (currentWord[findPos] == '"' || currentWord[findPos] == '\'')											// String highlights are open until the next string marker of the same type is found
	{
		posOffset += findPos;
		size_t startCol = posOffset;
		size_t startRow = i;
		currentWord = currentWord.substr(findPos);
		findEndMarker(fileRows, currentWord, i, posOffset, findPos, startRow, startCol, std::string() + currentWord[0], HighlightType::String);
	}
	else if (findPos + multilineCommentLength - 1 < currentWord.length() // Multiline comments stay open until the closing marker is found
				&& currentWord.substr(findPos, multilineCommentLength) == mCurrentSyntax->multilineCommentStart)
	{
		posOffset += findPos;
		size_t startCol = posOffset;
		size_t startRow = i;
		currentWord = currentWord.substr(findPos);
		findEndMarker(fileRows, currentWord, i, posOffset, findPos, startRow, startCol, mCurrentSyntax->multilineCommentEnd, HighlightType::MultilineComment);
	}
	else if (findPos + singlelineCommentLength - 1 < currentWord.length() // Singleline comments take the rest of the row
				&& currentWord.substr(findPos, singlelineCommentLength) == mCurrentSyntax->singlelineComment)
	{
		mHighlights.emplace_back(HighlightType::Comment, i, findPos + posOffset, i, row->renderedLine.length());
		gotoNextRow = true;
	}
	else
	{
		// Go to the next word
		posOffset += findPos + 1;
		currentWord = currentWord.substr(findPos + 1);
	}
	return gotoNextRow;
}

void SyntaxHighlight::highlightKeywordNumberCheck(std::string_view& currentWord, size_t i, size_t posOffset)
{
	if (currentWord.find_first_not_of("0123456789") == std::string::npos)
	{
		mHighlights.emplace_back(HighlightType::Number, i, posOffset, i, posOffset + currentWord.length());
		return;
	}
	else
	{
		if (mCurrentSyntax->builtInTypeKeywords.contains(std::string(currentWord)))
		{
			mHighlights.emplace_back(HighlightType::KeywordBuiltInType, i, posOffset, i, posOffset + currentWord.length());
			return;
		}
		else if (mCurrentSyntax->controlKeywords.contains(std::string(currentWord)))
		{
			mHighlights.emplace_back(HighlightType::KeywordControl, i, posOffset, i, posOffset + currentWord.length());
			return;
		}
		else if (mCurrentSyntax->otherKeywords.contains(std::string(currentWord)))
		{
			mHighlights.emplace_back(HighlightType::KeywordOther, i, posOffset, i, posOffset + currentWord.length());
			return;
		}
	}
}

void SyntaxHighlight::eraseHighlight(const size_t i)
{
	mHighlights.erase(mHighlights.begin() + i);
}

std::tuple<size_t, size_t, size_t> SyntaxHighlight::removeOffScreenHighlights(size_t rowOffset, size_t rows, size_t fileCursorY)
{
	size_t rowToStart = std::numeric_limits<size_t>::max();
	size_t colToStart = std::numeric_limits<size_t>::max();
	size_t rowToEnd = std::numeric_limits<size_t>::max();

	for (size_t i = 0; i < mHighlights.size(); ++i) // First pass gets rid of all unnecessary syntax highlights (all the off-screen ones)
	{
		HighlightLocation& highlight = mHighlights[i];
		if (highlight.highlightType != HighlightType::String && highlight.highlightType != HighlightType::MultilineComment)
		{
			eraseHighlight(i);
		}
		else
		{
			if (highlight.startRow < rowOffset && highlight.endRow < rowOffset)
			{
				highlight.drawColor = false;
				continue;
			}
			else if (highlight.startRow >= rowOffset && highlight.startRow < rowOffset + rows)
			{
				if (highlight.endRow >= rowOffset + rows) rowToEnd = highlight.endRow;
				eraseHighlight(i);
			}
			else if (highlight.endRow >= rowOffset && highlight.endRow < rowOffset + rows)
			{
				if (highlight.startRow < rowOffset && rowToStart == std::numeric_limits<size_t>::max())
				{
					rowToStart = highlight.startRow;
					colToStart = highlight.startCol;
				}
				eraseHighlight(i);
			}
			else if (highlight.endRow >= rowOffset + rows && highlight.startRow < rowOffset)
			{
				if (rowToStart == std::numeric_limits<size_t>::max())
				{
					rowToStart = highlight.startRow;
					colToStart = highlight.startCol;
					if (rowToEnd == std::numeric_limits<size_t>::max() || highlight.endRow > rowToEnd) rowToEnd = highlight.endRow;
					eraseHighlight(i);
				}
			}
			else if (!highlight.endFound)
			{
				if (highlight.startRow < rowOffset && rowToStart == std::numeric_limits<size_t>::max())
				{
					rowToStart = highlight.startRow;
					colToStart = highlight.startCol;
				}
				eraseHighlight(i);
			}
			else if (highlight.startRow >= rowOffset + rows) //Erase all remaining highlights
			{
				mHighlights.erase(mHighlights.begin() + i, mHighlights.end());
			}
		}

		--i;
	}

	if (mHighlights.size() > 0 && (mHighlights.back().endRow > rowToEnd || rowToEnd == std::numeric_limits<size_t>::max()))
	{
		rowToEnd = mHighlights.back().endRow;
	}
	return std::tuple<size_t, size_t, size_t>(rowToStart, colToStart, rowToEnd);
}