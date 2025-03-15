#include <gtest/gtest.h>
#include <sstream>

#include "Editor/Editor.hpp"
#include "MockConsole.hpp"

TEST(EditorTests, EditorInitializesSuccessfully)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
}

TEST(EditorTests, EditorRendersText)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	testing::internal::CaptureStdout();
	editor.refreshScreen(true);
	EXPECT_NE(testing::internal::GetCapturedStdout(), std::string()) << "Testing to make sure refreshScreen prints to the console";
}

TEST(EditorTests, EditorModeChangesAsExpected)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
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
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	bool notDirty = editor.isDirty();
	EXPECT_FALSE(notDirty);
	editor.insertChar('c');
	bool isDirty = editor.isDirty();
	EXPECT_TRUE(isDirty);
}

TEST(EditorTests, FileBecomesNotDirtyAfterSave)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	editor.insertChar('c');
	bool dirty = editor.isDirty();
	EXPECT_TRUE(dirty);
	editor.save();
	bool notDirty = editor.isDirty();
	EXPECT_FALSE(notDirty);
}

TEST(EditorTests, insertRowAddsNewRow)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	const std::vector<FileHandler::Row> beforeRowAddition = *editor.getWindowForTesting().fileRows;

	editor.addRow();

	const std::vector<FileHandler::Row> afterRowAddition = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeRowAddition, afterRowAddition) << "Adding row should change the output";
}

TEST(EditorTests, UndoRemovesInsertedCharAsExpected)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	const std::vector<FileHandler::Row> beforeChange = *editor.getWindowForTesting().fileRows;

	editor.insertChar('x');

	const std::vector<FileHandler::Row> afterInsert = *editor.getWindowForTesting().fileRows;

	editor.undoChange();

	const std::vector<FileHandler::Row> afterUndo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterInsert);
	EXPECT_NE(afterInsert, afterUndo);
	EXPECT_EQ(beforeChange, afterUndo);
}

TEST(EditorTests, RedoReinsertsCharAsExpected)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	const std::vector<FileHandler::Row> beforeChange = *editor.getWindowForTesting().fileRows;

	editor.insertChar('x');

	const std::vector<FileHandler::Row> afterInsert = *editor.getWindowForTesting().fileRows;

	editor.undoChange();

	const std::vector<FileHandler::Row> afterUndo = *editor.getWindowForTesting().fileRows;

	editor.redoChange();
	
	const std::vector<FileHandler::Row> afterRedo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterInsert);
	EXPECT_NE(afterInsert, afterUndo);
	EXPECT_NE(afterUndo, afterRedo);
	EXPECT_EQ(beforeChange, afterUndo);
	EXPECT_EQ(afterRedo, afterInsert);
}

TEST(EditorTests, UndoRemovesInsertedRowAsExpected)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	const std::vector<FileHandler::Row> beforeChange = *editor.getWindowForTesting().fileRows;

	editor.addRow();

	const std::vector<FileHandler::Row> afterInsert = *editor.getWindowForTesting().fileRows;

	editor.undoChange();

	const std::vector<FileHandler::Row> afterUndo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterInsert);
	EXPECT_NE(afterInsert, afterUndo);
	EXPECT_EQ(beforeChange, afterUndo);
}

TEST(EditorTests, RedoReinsertsRowAsExpected)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	const std::vector<FileHandler::Row> beforeChange = *editor.getWindowForTesting().fileRows;

	editor.addRow();

	const std::vector<FileHandler::Row> afterInsert = *editor.getWindowForTesting().fileRows;

	editor.undoChange();

	const std::vector<FileHandler::Row> afterUndo = *editor.getWindowForTesting().fileRows;

	editor.redoChange();
	
	const std::vector<FileHandler::Row> afterRedo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterInsert);
	EXPECT_NE(afterInsert, afterUndo);
	EXPECT_NE(afterUndo, afterRedo);
	EXPECT_EQ(beforeChange, afterUndo);
	EXPECT_EQ(afterRedo, afterInsert);
}

TEST(EditorTests, UndoReinsertsDeletedRowAsExpected)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	const std::vector<FileHandler::Row> beforeChange = *editor.getWindowForTesting().fileRows;

	editor.moveCursor(KeyActions::KeyAction::ArrowDown);
	editor.deleteChar(KeyActions::KeyAction::Backspace);
	std::vector<FileHandler::Row> afterDelete = *editor.getWindowForTesting().fileRows;

	editor.undoChange();
	std::vector<FileHandler::Row> afterUndo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterDelete);
	EXPECT_NE(afterDelete, afterUndo);
	EXPECT_EQ(beforeChange, afterUndo);

	editor.undoChange();
	editor.undoChange();

	editor.moveCursor(KeyActions::KeyAction::ArrowLeft);
	editor.deleteChar(KeyActions::KeyAction::Delete);

	afterDelete = *editor.getWindowForTesting().fileRows;

	editor.undoChange();
	afterUndo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterDelete);
	EXPECT_NE(afterDelete, afterUndo);
	EXPECT_EQ(beforeChange, afterUndo);
}

TEST(EditorTests, RedoRemovesDeletedRowAsExpected)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	const std::vector<FileHandler::Row> beforeChange = *editor.getWindowForTesting().fileRows;

	editor.moveCursor(KeyActions::KeyAction::ArrowDown);
	editor.deleteChar(KeyActions::KeyAction::Backspace);
	std::vector<FileHandler::Row> afterDelete = *editor.getWindowForTesting().fileRows;

	editor.undoChange();
	std::vector<FileHandler::Row> afterUndo = *editor.getWindowForTesting().fileRows;

	editor.redoChange();
	std::vector<FileHandler::Row> afterRedo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterDelete);
	EXPECT_NE(afterDelete, afterUndo);
	EXPECT_NE(afterUndo, afterRedo);
	EXPECT_EQ(beforeChange, afterUndo);
	EXPECT_EQ(afterRedo, afterDelete);

	editor.undoChange();
	editor.undoChange();

	editor.moveCursor(KeyActions::KeyAction::ArrowLeft);
	editor.deleteChar(KeyActions::KeyAction::Delete);

	afterDelete = *editor.getWindowForTesting().fileRows;

	editor.undoChange();
	afterUndo = *editor.getWindowForTesting().fileRows;

	editor.redoChange();
	afterRedo = *editor.getWindowForTesting().fileRows;

	EXPECT_NE(beforeChange, afterDelete);
	EXPECT_NE(afterDelete, afterUndo);
	EXPECT_NE(afterUndo, afterRedo);
	EXPECT_EQ(beforeChange, afterUndo);
	EXPECT_EQ(afterRedo, afterDelete);
}

TEST(EditorTests, FindStringMovesCursor)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));

	size_t fileCursorX = editor.getWindowForTesting().fileCursorX;
	size_t fileCursorY = editor.getWindowForTesting().fileCursorY;

	editor.findString("just");

	size_t fileCursorXFind = editor.getWindowForTesting().fileCursorX;
	size_t fileCursorYFind = editor.getWindowForTesting().fileCursorY;

	EXPECT_NE(fileCursorX + fileCursorY, fileCursorXFind + fileCursorYFind);
	EXPECT_EQ(fileCursorYFind, 1);
	EXPECT_EQ(fileCursorXFind, 0);
}

TEST(EditorTests, MoveCursorToFindWorks)
{
	Editor editor(SyntaxHighlight(".txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));

	size_t fileCursorX = editor.getWindowForTesting().fileCursorX;
	size_t fileCursorY = editor.getWindowForTesting().fileCursorY;

	editor.findString("just");

	size_t fileCursorXFind = editor.getWindowForTesting().fileCursorX;
	size_t fileCursorYFind = editor.getWindowForTesting().fileCursorY;

	EXPECT_NE(fileCursorX + fileCursorY, fileCursorXFind + fileCursorYFind);
	EXPECT_EQ(fileCursorYFind, 1);
	EXPECT_EQ(fileCursorXFind, 0);

	editor.moveCursorToFind(KeyActions::KeyAction::Enter);

	fileCursorXFind = editor.getWindowForTesting().fileCursorX;
	fileCursorYFind = editor.getWindowForTesting().fileCursorY;

	EXPECT_EQ(fileCursorYFind, 2);
	EXPECT_EQ(fileCursorXFind, 1);
}