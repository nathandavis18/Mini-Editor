#include <gtest/gtest.h>
#include <vector>

#include "FindAndReplace/FindAndReplace.hpp"
#include "File/File.hpp"

TEST(FindAndReplaceTests, FindReturnsCorrectAmount)
{
	std::vector<FileHandler::Row> rows{
		FileHandler::Row("test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r2test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r3test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r4test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r5test, test2, t3st3, test4, otherword, otherwordwithtest")
	};
	std::vector<FindAndReplace::FindLocation> locations = FindAndReplace::find("test", rows);

	EXPECT_EQ(locations.size(), 20);
}

TEST(FindAndReplaceTests, FindReturnsEmptyWithNoMatch)
{
	std::vector<FileHandler::Row> rows{
		FileHandler::Row("test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r2test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r3test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r4test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r5test, test2, t3st3, test4, otherword, otherwordwithtest")
	};
	std::vector<FindAndReplace::FindLocation> locations = FindAndReplace::find("nomatch", rows);

	EXPECT_EQ(locations.size(), 0);
}

TEST(FindAndReplaceTests, ReplaceReplacesCorrectOne)
{
	std::vector<FileHandler::Row> rows{
		FileHandler::Row("test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r2test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r3test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r4test, test2, t3st3, test4, otherword, otherwordwithtest"),
		FileHandler::Row("r5test, test2, t3st3, test4, otherword, otherwordwithtest")
	};
	std::vector<FindAndReplace::FindLocation> locations = FindAndReplace::find("test", rows);

	FindAndReplace::replace(rows[0].line, "replacedTest", locations[0]);

	EXPECT_EQ(rows[0].line, "replacedTest, test2, t3st3, test4, otherword, otherwordwithtest");
}