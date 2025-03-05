#include <gtest/gtest.h>
#include <sstream>

#include "Editor/Editor.hpp"
#include "MockConsole.hpp"

TEST(EditorTests, EditorInitializesSuccessfully)
{
	SyntaxHighlight highlight("testFile.txt");
	FileHandler file("testFile.txt");
	Editor editor(std::move(highlight), std::move(file), std::make_unique<MockConsole>(MockConsole()));
}

TEST(EditorTests, EditorRendersText)
{
	SyntaxHighlight highlight("testFile.txt");
	FileHandler file("testFile.txt");
	Editor editor(std::move(highlight), std::move(file), std::make_unique<MockConsole>(MockConsole()));
	testing::internal::CaptureStdout();
	editor.refreshScreen(true);
	EXPECT_NE(testing::internal::GetCapturedStdout(), std::string()) << "Testing to make sure refreshScreen prints to the console";
}

TEST(EditorTests, EditorModeChangesAsExpected)
{
	SyntaxHighlight highlight("testFile.txt");
	FileHandler file("testFile.txt");
	Editor editor(std::move(highlight), std::move(file), std::make_unique<MockConsole>(MockConsole()));
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
	SyntaxHighlight highlight("testFile.txt");
	FileHandler file("testFile.txt");
	Editor editor(std::move(highlight), std::move(file), std::make_unique<MockConsole>(MockConsole()));
	bool notDirty = editor.isDirty();
	EXPECT_FALSE(notDirty);
	editor.insertChar('c');
	bool isDirty = editor.isDirty();
	EXPECT_TRUE(isDirty);
}

TEST(EditorTests, insertRowAddsNewRow)
{
	SyntaxHighlight highlight("testFile.txt");
	FileHandler file("testFile.txt");
	Editor editor(std::move(highlight), std::move(file), std::make_unique<MockConsole>(MockConsole()));
	testing::internal::CaptureStdout();
	editor.refreshScreen(true);
	std::string beforeRowAddition = testing::internal::GetCapturedStdout();

	editor.addRow();

	testing::internal::CaptureStdout();
	editor.refreshScreen(true);
	std::string afterRowAddition = testing::internal::GetCapturedStdout();

	EXPECT_NE(beforeRowAddition, afterRowAddition) << "Adding row should change the output";
}