#include <gtest/gtest.h>
#include <cstdint>
#include <tuple>
#include <limits>

#include "SyntaxHighlight/SyntaxHighlight.hpp"

TEST(SyntaxHighlightTests, TextFileHasNoSyntax)
{
	SyntaxHighlight::initSyntax("TestFile.txt");
	EXPECT_FALSE(SyntaxHighlight::hasSyntax());
}

TEST(SyntaxHighlightTests, CppFileHasSyntax)
{
	SyntaxHighlight::initSyntax("TestFile.cpp");
	EXPECT_TRUE(SyntaxHighlight::hasSyntax());
}

TEST(SyntaxHighlightTests, ColorValuesReturnCorrectly)
{
	constexpr uint8_t white = 255;
	uint8_t color = SyntaxHighlight::color(SyntaxHighlight::HighlightType::Normal);
	EXPECT_EQ(white, color);
}

TEST(SyntaxHighlightTests, RemoveHighlightsReturnsCorretlyWithNoHighlights)
{
	std::tuple<size_t, size_t> testValue(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());
	std::tuple<size_t, size_t> returnValue = SyntaxHighlight::removeOffScreenHighlights(0, 0, 0);
	EXPECT_EQ(testValue, returnValue);
}
