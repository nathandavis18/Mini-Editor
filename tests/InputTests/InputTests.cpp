#include <gtest/gtest.h>
#include <iostream>

#include "Editor/Editor.hpp"
#include "Input/Input.hpp"

TEST(InputTests, EnableEditModeWorks)
{
	Editor editor("TestFile.cpp");
	InputHandler::changeMode(static_cast<KeyActions::KeyAction>('i'), editor);
	EXPECT_EQ(editor.mode(), Editor::Mode::EditMode);
}

TEST(InputTests, EnableReadModeWorks)
{
	Editor editor("TestFile.cpp");
	InputHandler::handleInput(KeyActions::KeyAction::Esc, editor);
	EXPECT_EQ(editor.mode(), Editor::Mode::ReadMode);
}

TEST(InputTests, InsertCharChangesFile)
{
	Editor editor("TestFile.cpp");
	bool beforeChange = editor.isDirty();
	editor.enableEditMode();
	InputHandler::handleInput(static_cast<KeyActions::KeyAction>('c'), editor);
	bool afterChange = editor.isDirty();
	EXPECT_NE(beforeChange, afterChange);
}