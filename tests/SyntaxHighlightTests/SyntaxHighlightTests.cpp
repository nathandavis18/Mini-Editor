#include <gtest/gtest.h>
#include <cstdint>
#include <tuple>
#include <limits>

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

TEST(SyntaxHighlightTests, ColorValuesReturnCorrectly)
{
	constexpr uint8_t white = 15;
	SyntaxHighlight highlight("TestFile.cpp");
	uint8_t color = highlight.color(SyntaxHighlight::HighlightType::Normal);
	EXPECT_EQ(white, color);
}

TEST(SyntaxHighlightTests, RemoveHighlightsReturnsCorretlyWithNoHighlights)
{
	std::tuple<size_t, size_t> testValue(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());
	SyntaxHighlight highlight("TestFile.cpp");
	std::tuple<size_t, size_t> returnValue = highlight.removeOffScreenHighlights(0, 0, 0);
	EXPECT_EQ(testValue, returnValue);
}
