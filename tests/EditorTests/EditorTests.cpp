#include <gtest/gtest.h>
#include <sstream>
#include <iostream>

#include "Editor/Editor.hpp"
#include "File/File.hpp"
#include "Input/Input.hpp"

TEST(EditorTests, EditorInitializesSuccessfully)
{
	Editor editor("testFile.txt");
	EXPECT_NO_FATAL_FAILURE();
}

TEST(EditorTests, EditorRendersText)
{
	Editor editor("testFile.txt");
	testing::internal::CaptureStdout();
	editor.refreshScreen(true);
	EXPECT_NE(testing::internal::GetCapturedStdout(), std::string()) << "Testing to make sure refreshScreen prints to the console";
}

TEST(EditorTests, EditorModeChangesAsExpected)
{
	Editor editor("testFile.txt");
	editor.enableCommandMode();
	EXPECT_EQ(editor.mode(), Editor::Mode::CommandMode);

	editor.enableEditMode();
	EXPECT_EQ(editor.mode(), Editor::Mode::EditMode);

	editor.enableReadMode();
	EXPECT_EQ(editor.mode(), Editor::Mode::ReadMode);

	editor.enableExitMode();
	EXPECT_EQ(editor.mode(), Editor::Mode::ExitMode);
}

TEST(EditorTests, FileBecomesDirtyAfterChange)
{
	Editor editor("testFile.txt");
	bool notDirty = editor.isDirty();
	EXPECT_FALSE(notDirty);
	editor.insertChar('c');
	bool isDirty = editor.isDirty();
	EXPECT_TRUE(isDirty);
}

TEST(EditorTests, insertRowAddsNewRow)
{
	Editor editor("testFile.txt");
	testing::internal::CaptureStdout();
	editor.refreshScreen(true);
	std::string beforeRowAddition = testing::internal::GetCapturedStdout();

	editor.addRow();

	testing::internal::CaptureStdout();
	editor.refreshScreen(true);
	std::string afterRowAddition = testing::internal::GetCapturedStdout();

	EXPECT_NE(beforeRowAddition, afterRowAddition) << "Adding row should change the output";
}