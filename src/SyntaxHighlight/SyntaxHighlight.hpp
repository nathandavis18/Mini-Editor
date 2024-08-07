#pragma once
#include <vector>
#include <string>
namespace SyntaxHighlight{
	struct EditorSyntax
	{
		std::vector<std::string> filematch;
		std::vector<std::string> keywords;
		std::string singlelineComment;
		std::string multilineCommentStart;
		std::string multilineCommentEnd;
	};
	void addSyntax(const std::vector<std::string>& filetypes, const std::vector<std::string>& keywords, const std::string& singlelineComment,
		const std::string& multilineCommentStart, const std::string& multilineCommentEnd);

	EditorSyntax& syntax();

	void initSyntax(const std::string_view& fName);

	enum class HighlightType
	{
		Normal,
		Comment,
		MultilineComment,
		KeywordType,
		KeywordLoop,
		KeywordOther,
		String,
		Number,
		Find
	};
	struct HighlightColor
	{
		int r, g, b;
	};

	static const std::vector<std::string> cppFiletypes{ ".cpp", ".cc", ".cxx", ".hpp", ".h", ".hxx", ".hh" };
	static const std::vector<std::string> cppKeywords{
		//Types
		"int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
		"void|", "short|", "auto|", "const|", "bool|", "enum|", "nullptr|",
		"struct", "class", "constexpr", "volatile", "mutable", "union", "typedef",
		"#define", "consteval", "register", "compl", "explicit", "true", "false", "virtual",

		//Loop/Control keywords
		"for", "while", "do", "continue", "break", "if", "else", "not", "not_eq",
		"or", "or_eq", "throw", "catch", "try", "xor", "xor_eq", "goto", "return",
		"bitand", "bitor", "case",

		//Other keywords
		"decltype", "sizeof", "static_cast", "dynamic_cast", "reinterpret_cast", "template", "this",
		"operator", "private", "pubic", "protected", "inline", "typeid", "typename", "alignas", "alignof", "export"
	};
}