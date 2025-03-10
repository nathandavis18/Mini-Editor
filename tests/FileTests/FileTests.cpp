#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>

#include "File/File.hpp"

TEST(FileTest, FileNameGetsSavedProperly)
{
	FileHandler file("TestFileName.txt");
	EXPECT_EQ(file.fileName(), "TestFileName.txt") << "The file name should be set to TestFileName.txt";
}

TEST(FileTest, FileReturnsCorrectNumRows)
{
	FileHandler file("testFile.txt");
	std::vector<FileHandler::Row> rows = *file.getFileContents();
	EXPECT_EQ(rows.size(), 4) << "rows.size() should return 4";
}

TEST(FileTest, FileReturnsEmptyWhenFileDoesntExist)
{
	FileHandler file("nonexistantFile.txt");
	std::vector<FileHandler::Row> rows = *file.getFileContents();
	EXPECT_EQ(rows.size(), 0) << "New/Nonexistant File should not have any rows";
}

TEST(FileTest, SavingFileDoesntCorruptFile)
{
	FileHandler fileHandler("testFile.txt");

	std::stringstream stream1;
	std::ifstream file("testFile.txt");
	stream1 << file.rdbuf();
	file.close();

	fileHandler.saveFile();

	std::stringstream stream2;
	file.open("testFile.txt");
	stream2 << file.rdbuf();
	file.close();

	EXPECT_EQ(stream1.str(), stream2.str()) << "File contents shouldn't be corrupt after saving";
}

TEST(FileTest, LargeFileOpensQuickly)
{
	std::chrono::steady_clock::time_point before = std::chrono::steady_clock::now();
	FileHandler fileHandler("test.cpp"); //This is a file with > 8 million lines, all getting parsed properly
	std::chrono::steady_clock::time_point after = std::chrono::steady_clock::now();

	std::chrono::milliseconds expectedMaxTime(5000); //Want it to open and parse in < 5 seconds.

	std::chrono::milliseconds actualTime = std::chrono::duration_cast<std::chrono::milliseconds>(after - before);

	EXPECT_TRUE(actualTime <= expectedMaxTime);
}