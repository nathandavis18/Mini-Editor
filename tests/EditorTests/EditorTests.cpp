#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

#include "Editor/Editor.hpp"
#include "File/File.hpp"
#include "Input/Input.hpp"

TEST(EditorTests, EditorInitializesSuccessfully)
{
	Editor::initEditor("testFile.txt");
	EXPECT_NO_FATAL_FAILURE();
}

TEST(EditorTests, EditorRendersText)
{
	testing::internal::CaptureStdout();
	Editor::refreshScreen(true);
	EXPECT_NE(testing::internal::GetCapturedStdout(), std::string()) << "Testing to make sure refreshScreen prints to the console";
}

TEST(EditorTests, EditorModeChangesAsExpected)
{
	Editor::enableCommandMode();
	EXPECT_EQ(Editor::mode(), Editor::Mode::CommandMode);

	Editor::enableEditMode();
	EXPECT_EQ(Editor::mode(), Editor::Mode::EditMode);

	Editor::enableReadMode();
	EXPECT_EQ(Editor::mode(), Editor::Mode::ReadMode);

	Editor::enableExitMode();
	EXPECT_EQ(Editor::mode(), Editor::Mode::ExitMode);
}

TEST(EditorTests, FileBecomesDirtyAfterChange)
{
	bool notDirty = Editor::isDirty();
	EXPECT_FALSE(notDirty);
	Editor::insertChar('c');
	bool isDirty = Editor::isDirty();
	EXPECT_TRUE(isDirty);
}

TEST(EditorTests, insertRowAddsNewRow)
{
	testing::internal::CaptureStdout();
	Editor::refreshScreen(true);
	std::string beforeRowAddition = testing::internal::GetCapturedStdout();

	Editor::addRow();

	testing::internal::CaptureStdout();
	Editor::refreshScreen(true);
	std::string afterRowAddition = testing::internal::GetCapturedStdout();

	EXPECT_NE(beforeRowAddition, afterRowAddition) << "Adding row should change the output";
}