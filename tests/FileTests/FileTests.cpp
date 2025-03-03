#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <sstream>

#include "File/File.hpp"
#include "Editor/Editor.hpp"

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
	Editor editor("testFile.txt");
	std::stringstream stream1;
	std::ifstream file("testFile.txt");
	stream1 << file.rdbuf();
	file.close();

	editor.save();

	std::stringstream stream2;
	file.open("testFile.txt");
	stream2 << file.rdbuf();
	file.close();

	EXPECT_EQ(stream1.str(), stream2.str()) << "File contents shouldn't be corrupt after saving";
}