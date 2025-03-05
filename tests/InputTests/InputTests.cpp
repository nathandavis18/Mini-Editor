#include <gtest/gtest.h>

#include "Editor/Editor.hpp"
#include "Input/Input.hpp"
#include "MockConsole.hpp"

TEST(InputTests, EnableEditModeWorks)
{
	MockConsole console;
	SyntaxHighlight highlight("testFile.cpp");
	FileHandler file("testFile.cpp");
	Editor editor(std::move(highlight), std::move(file), std::move(console));
	InputHandler::changeMode(static_cast<KeyActions::KeyAction>('i'), editor);
	EXPECT_EQ(editor.mode(), Editor::Mode::EditMode);
}

TEST(InputTests, EnableReadModeWorks)
{
	MockConsole console;
	SyntaxHighlight highlight("testFile.cpp");
	FileHandler file("testFile.cpp");
	Editor editor(std::move(highlight), std::move(file), std::move(console));
	InputHandler::handleInput(KeyActions::KeyAction::Esc, editor);
	EXPECT_EQ(editor.mode(), Editor::Mode::ReadMode);
}

TEST(InputTests, InsertCharChangesFile)
{
	MockConsole console;
	SyntaxHighlight highlight("testFile.cpp");
	FileHandler file("testFile.cpp");
	Editor editor(std::move(highlight), std::move(file), std::move(console));

	bool beforeChange = editor.isDirty();
	editor.enableEditMode();
	InputHandler::handleInput(static_cast<KeyActions::KeyAction>('c'), editor);
	bool afterChange = editor.isDirty();
	EXPECT_NE(beforeChange, afterChange);
}