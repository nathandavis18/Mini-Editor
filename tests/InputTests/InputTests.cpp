#include <gtest/gtest.h>

#include "Editor/Editor.hpp"
#include "Input/Input.hpp"
#include "MockConsole.hpp"

TEST(InputTests, EnableEditModeWorks)
{
	Editor editor(SyntaxHighlight("testFile.txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	InputHandler::changeMode(static_cast<KeyActions::KeyAction>('i'), editor);
	EXPECT_EQ(editor.mode(), Editor::Mode::EditMode);
}

TEST(InputTests, EnableReadModeWorks)
{
	Editor editor(SyntaxHighlight("testFile.txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));
	InputHandler::handleInput(KeyActions::KeyAction::Esc, editor);
	EXPECT_EQ(editor.mode(), Editor::Mode::ReadMode);
}

TEST(InputTests, InsertCharChangesFile)
{
	Editor editor(SyntaxHighlight("testFile.txt"), FileHandler("testFile.txt"), std::make_unique<MockConsole>(MockConsole()));

	bool beforeChange = editor.isDirty();
	editor.enableEditMode();
	InputHandler::handleInput(static_cast<KeyActions::KeyAction>('c'), editor);
	bool afterChange = editor.isDirty();
	EXPECT_NE(beforeChange, afterChange);
}