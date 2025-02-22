#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <sstream>

#include "File/File.hpp"
#include "Editor/Editor.hpp"

TEST(FileTest, FileNameGetsSavedProperly)
{
	FileHandler::initFileHandler("TestFileName.txt");
	EXPECT_EQ(FileHandler::fileName(), "TestFileName.txt") << "The file name should be set to TestFileName.txt";
}

TEST(FileTest, FileReturnsCorrectNumRows)
{
	FileHandler::initFileHandler("testFile.txt");
	std::vector<FileHandler::Row> rows = FileHandler::getFileContents();
	EXPECT_EQ(rows.size(), 4) << "rows.size() should return 4";
}

TEST(FileTest, FileReturnsEmptyWhenFileDoesntExist)
{
	FileHandler::initFileHandler("nonexistantFile.txt");
	std::vector<FileHandler::Row> rows = FileHandler::getFileContents();
	EXPECT_EQ(rows.size(), 0) << "New/Nonexistant File should not have any rows";
}

TEST(FileTest, SavingFileDoesntCorruptFile)
{
	Editor::initEditor("testFile.txt");
	std::stringstream stream1;
	std::ifstream file("testFile.txt");
	stream1 << file.rdbuf();
	file.close();

	Editor::save();

	std::stringstream stream2;
	file.open("testFile.txt");
	stream2 << file.rdbuf();
	file.close();

	EXPECT_EQ(stream1.str(), stream2.str()) << "File contents shouldn't be corrupt after saving";
}