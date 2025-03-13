#include <gtest/gtest.h>
#include <vector>

#include "FindString/FindString.hpp"
#include "File/File.hpp"

TEST(FindStringTests, FindStringReturnsCorrectAmount)
{
	std::vector<FileHandler::Row> rows{
		FileHandler::Row("test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r2test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r3test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r4test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r5test, test2, t3st3, test4, otherword, otherwordwithtest")
	};
	std::vector<FindString::FindLocation> locations = FindString::find("test", rows);

	EXPECT_EQ(locations.size(), 20);
}

TEST(FindStringTests, FindStringReturnsEmptyWithNoMatch)
{
	std::vector<FileHandler::Row> rows{
		FileHandler::Row("test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r2test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r3test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r4test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r5test, test2, t3st3, test4, otherword, otherwordwithtest")
	};
	std::vector<FindString::FindLocation> locations = FindString::find("nomatch", rows);

	EXPECT_EQ(locations.size(), 0);
}