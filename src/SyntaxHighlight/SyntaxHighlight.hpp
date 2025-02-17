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

#pragma once
#include <File/File.hpp>

#include <vector>
#include <string>
#include <string_view>
#include <cstdint> //uint8_t
#include <unordered_set>

namespace SyntaxHighlight{

	struct EditorSyntax
	{
		std::unordered_set<std::string_view> filematch;
		std::unordered_set<std::string_view> builtInTypeKeywords;
		std::unordered_set<std::string_view> loopKeywords;
		std::unordered_set<std::string_view> otherKeywords;
		std::string_view singlelineComment;
		std::string_view multilineCommentStart;
		std::string_view multilineCommentEnd;
		char escapeChar;
	};

	const EditorSyntax* syntax();

	void initSyntax(const std::string_view& fName);


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
		EnumCount
	};

	uint8_t color(HighlightType);

	struct HighlightLocations
	{
		HighlightType highlightType;
		size_t startRow, startCol, endRow, endCol;
		bool endFound = true, drawColor = true;
	};
	
	std::vector<HighlightLocations>& highlightLocations();
	void findEndMarker(std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, size_t& row, size_t& posOffset, size_t& findPos, size_t startRow, size_t startCol, const std::string_view& strToFind, const HighlightType);
	bool highlightCommentCheck(std::vector<FileHandler::Row>& fileRows, std::string_view& currentWord, FileHandler::Row* row, size_t findPos, size_t& posOffset, size_t& i);
	std::tuple<int64_t, int64_t> removeOffScreenHighlights(size_t rowOffset, size_t rows, size_t fileCursorY);
	void highlightKeywordNumberCheck(std::string_view& currentWord, size_t i, size_t posOffset);

	//================================================ CPP KEYWORDS =================================================================\\

	static const std::unordered_set<std::string_view> cppFiletypes{ ".cpp", ".cc", ".cxx", ".hpp", ".h", ".hxx", ".hh" };
	static const std::unordered_set<std::string_view> cppBuiltInTypes{
		//Built-in types and main keywords
		"alignas", "alignof", "asm", "_asm", "auto", "bool", "char", "char8_t", "char16_t", "char32_t", "class",
		"compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast", "decltype", "delete", "double",
		"dynamic_cast", "enum", "explicit", "export", "extern", "false", "float", "friend", "inline", "int", "long",
		"mutable", "namespace", "new", "noexcept", "nullptr", "operator", "private", "protected", "public", "register",
		"reinterpret_cast", "requires", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
		"template", "this", "thread_local", "true", "typedef", "typeid", "typename", "union", "unsigned", "using", "virtual",
		"void", "volatile", "wchar_t"
	};
	static const std::unordered_set<std::string_view> cppControlKeywords{
		//Loop/Control keywords
		"and", "and_eq", "bitand", "bitor", "break", "case", "catch", "continue", "co_await", "co_return", "co_yield", "default",
		"do", "else", "for", "goto", "if", "not", "not_eq", "or", "or_eq", "return", "switch", "throw", "try", "while", "xor", "xor_eq"
	};
	static const std::unordered_set<std::string_view> cppOtherKeywords{
		//Some other keywords, such as macro definitions
		"#define", "#ifdef", "#ifndef", "#if", "defined", "#include", "#elif", "#endif"
	}; 
}