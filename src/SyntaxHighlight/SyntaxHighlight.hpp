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
 * @file SyntaxHighlight.hpp
 * @brief Provides the interface for how syntax highlight is stored
 */

#pragma once
#include "File/File.hpp"
#include "Utility/JsonParser/JsonParser.hpp"

#include <vector>
#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include <unordered_set>

class SyntaxHighlight
{
public:
	/// <summary>
	/// The structure to store what file types match with each keyword, comment, or string type.
	/// </summary>
	struct EditorSyntax
	{
		std::unordered_set<std::string> filematch;
		std::unordered_set<std::string> builtInTypeKeywords;
		std::unordered_set<std::string> controlKeywords;
		std::unordered_set<std::string> otherKeywords;

		//These values may need a default/fallback value for when a syntax exists but doesn't define them
		std::string singlelineComment = "//";
		std::string multilineCommentStart = "/*";
		std::string multilineCommentEnd = "*/";
		char escapeChar = '\\';
	};

	/// <summary>
	/// Initializes the syntax highlight functionality.
	/// </summary>
	/// <param name="fName"></param>
	SyntaxHighlight(const std::string_view fName);

	/// <summary>
	/// Returns whether or not there is an active highlight syntax being used
	/// </summary>
	/// <returns></returns>
	const bool hasSyntax() const;

	/// <summary>
	/// The different types of highlights
	/// </summary>
	enum class HighlightType
	{
		Normal,
		Comment,
		MultilineComment,
		KeywordBuiltInType,
		KeywordControl,
		KeywordOther,
		String,
		Number,
		EnumCount // A hacky way to get the number of items in an enum if each item in enum is default assigned
	};

	/// <summary>
	/// Returns the color code of a specific highlight type
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	const uint8_t color(HighlightType hlType) const;

	/// <summary>
	/// The structure for how highlight locations are stored
	/// A custom token-based system for highlights
	/// </summary>
	struct HighlightLocations
	{
		HighlightType highlightType = HighlightType::Normal;
		size_t startRow = 0, startCol = 0, endRow = 0, endCol = 0;
		bool endFound = true, drawColor = true;
	};

	/// <summary>
	/// Returns a const reference to the internal highlight locations
	/// Should only be called one time by the Editor, and that is on program load
	/// </summary>
	/// <returns></returns>
	const std::vector<HighlightLocations>& highlights() const;

	/// <summary>
	/// Called when a multiline comment or string highlight location is started
	/// </summary>
	/// <param name="fileRows"></param>
	/// <param name="currentWord"></param>
	/// <param name="row"></param>
	/// <param name="posOffset"></param>
	/// <param name="findPos"></param>
	/// <param name="startRow"></param>
	/// <param name="startCol"></param>
	/// <param name="strToFind"></param>
	/// <param name=""></param>
	void findEndMarker(const std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, size_t& row, size_t& posOffset, size_t& findPos, size_t startRow, size_t startCol, const std::string_view& strToFind, const HighlightType);

	/// <summary>
	/// Checks the type of comment highlight currently found, if one is found
	/// </summary>
	/// <param name="fileRows"></param>
	/// <param name="currentWord"></param>
	/// <param name="row"></param>
	/// <param name="findPos"></param>
	/// <param name="posOffset"></param>
	/// <param name="i"></param>
	/// <returns></returns>
	bool highlightCommentCheck(const std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, FileHandler::Row* row, size_t findPos, size_t& posOffset, size_t& i);

	/// <summary>
	/// Removes all the un-needed highlights that are off-screen, and returns the position of the rowOffset to start checking for highlights on again.
	/// </summary>
	/// <param name="rowOffset"></param>
	/// <param name="rows"></param>
	/// <param name="fileCursorY"></param>
	/// <returns></returns>
	std::tuple<size_t, size_t> removeOffScreenHighlights(size_t rowOffset, size_t rows, size_t fileCursorY);

	/// <summary>
	/// Checks if the current word is a number or keyword, since they dont need any other special treatment like comments and strings
	/// </summary>
	/// <param name="currentWord"></param>
	/// <param name="i"></param>
	/// <param name="posOffset"></param>
	void highlightKeywordNumberCheck(std::string_view& currentWord, size_t i, size_t posOffset);


	private:
		/// <summary>
		/// Called on syntax initialization. Finds the active syntax, if one exists.
		/// Calls setColors() and setEditorSyntax() with the active syntax
		/// </summary>
		/// <param name="mp"></param>
		/// <param name="extension"></param>
		void setSyntax(const std::vector<JsonParser::JsonObject>& mp, const std::string& extension);

		/// <summary>
		/// Sets the colors for the syntax based on what keys are defined.
		/// Called on initialization of syntax only
		/// </summary>
		/// <param name="syntax"></param>
		void setColors(const JsonParser::JsonValue& syntax);

		/// <summary>
		/// Sets the editor syntax information, including keywords, comment identifiers, etc.
		/// </summary>
		/// <param name="syntax"></param>
		void setEditorSyntax(const JsonParser::JsonValue& syntax);

	private:
		// Color IDs correspond to the IDs found at this link: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797#:~:text=Where%20%7BID%7D%20should%20be%20replaced%20with%20the%20color%20index%20from%200%20to%20255%20of%20the%20following%20color%20table%3A
		// If you would like to add/change colors, just find the color ID you want and add it. There is a list of colors chosen here for your convenience
		static inline const std::unordered_map<std::string, uint8_t> mColorKeys{
			{"pink", 13}, {"magenta", 207}, {"hotpink", 5}, {"rosered", 204},
			{"lightred", 1}, {"red", 160}, {"darkred", 52}, {"darkorange", 130},
			{"peach", 209}, {"orange", 202}, {"lightorange", 208}, {"lightyellow", 11},
			{"marigoldyellow", 3}, {"yellow", 226}, {"darkyellow", 178}, {"darklimegreen", 2},
			{"lightgreen", 46}, {"green", 28}, {"darkgreen", 22}, {"tealgreen", 42},
			{"teal", 23}, {"tealblue", 6}, {"lightblue", 4}, {"seablue", 14}, {"blue", 20},
			{"navyblue", 17}, {"darkblue", 18}, {"purple", 93}, {"darkpurple", 57}, {"lightgray", 7},
			{"gray", 8}, {"white", 15}, {"black", 16}
		};

		std::array<uint8_t, static_cast<uint8_t>(HighlightType::EnumCount)> mColors;
		std::vector<HighlightLocations> mHighlights;
		std::unique_ptr<EditorSyntax> mCurrentSyntax;
		std::string mFileContents;
};