#include <gtest/gtest.h>
#include <cstdint>
#include <tuple>
#include <limits>
#include <string_view>
#include <vector>

#include "File/File.hpp"
#include "SyntaxHighlight/SyntaxHighlight.hpp"

TEST(SyntaxHighlightTests, TextFileHasNoSyntax)
{
	SyntaxHighlight highlight("TestFile.txt");
	EXPECT_FALSE(highlight.hasSyntax());
}

TEST(SyntaxHighlightTests, CppFileHasSyntax)
{
	SyntaxHighlight highlight("TestFile.cpp");
	EXPECT_TRUE(highlight.hasSyntax());
}

TEST(SyntaxHighlightTests, RemoveHighlightsReturnsCorretlyWithNoHighlights)
{
	std::tuple<size_t, size_t> testValue(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());
	SyntaxHighlight highlight("TestFile.cpp");
	std::tuple<size_t, size_t> returnValue = highlight.removeOffScreenHighlights(0, 0, 0);
	EXPECT_EQ(testValue, returnValue);
}

TEST(SyntaxHighlightTests, ColorCodeReturnsCorrectly)
{
	SyntaxHighlight highlight("TestFile.cpp");
	uint8_t expectedValue = 15;

	uint8_t actualValue = highlight.color(SyntaxHighlight::HighlightType::Normal);
	EXPECT_EQ(expectedValue, actualValue);
}

TEST(SyntaxHighlightTests, KeywordGetsAddedToHighlights)
{
	SyntaxHighlight highlight("TestFile.cpp");
	uint8_t numHighlights = highlight.highlights().size();
	EXPECT_EQ(numHighlights, 0);

	std::string_view keyword = "int";
	highlight.highlightKeywordNumberCheck(keyword, 0, 0);
	numHighlights = highlight.highlights().size();
	EXPECT_EQ(numHighlights, 1);
}

TEST(SyntaxHighlightTests, OffScreenHighlightsGetRemoved)
{
	SyntaxHighlight highlight("TestFile.cpp");
	std::string_view keyword = "int";
	highlight.highlightKeywordNumberCheck(keyword, 0, 0);
	uint8_t beforeRemoval = highlight.highlights().size();
	EXPECT_EQ(beforeRemoval, 1);

	highlight.removeOffScreenHighlights(15, 15, 15);
	uint8_t afterRemoval = highlight.highlights().size();
	EXPECT_EQ(afterRemoval, 0);
}

TEST(SyntaxHighlightTests, MultilineCommentCheckWorks)
{
	SyntaxHighlight highlight("TestFile.cpp");
	std::vector<FileHandler::Row> fileRows{
		FileHandler::Row("/*Start to a multilineComment", "/*Start to a multilineComment"),
		FileHandler::Row("End to a multilineComment*/", "End to a multilineComment*/"),
		FileHandler::Row("Test line at end to make sure end marker end row is correct", "Test line")
	};

	uint8_t beforeAddition = highlight.highlights().size();
	EXPECT_EQ(beforeAddition, 0);

	std::string_view currentWord = fileRows.at(0).renderedLine;
	size_t posOffset = 0;
	size_t i = 0;
	highlight.highlightCommentCheck(fileRows, currentWord, &fileRows.at(0), 0, posOffset, i);

	uint8_t afterAddition = highlight.highlights().size();
	EXPECT_EQ(afterAddition, 1);

	EXPECT_EQ(highlight.highlights().at(0).startRow, 0);
	EXPECT_EQ(highlight.highlights().at(0).endRow, 1);
	EXPECT_EQ(highlight.highlights().at(0).highlightType, SyntaxHighlight::HighlightType::MultilineComment);
}
