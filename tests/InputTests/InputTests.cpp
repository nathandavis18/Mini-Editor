#include <gtest/gtest.h>
#include <iostream>

#include "Editor/Editor.hpp"
#include "Input/Input.hpp"

TEST(InputTests, EnableInputModeWorks)
{
	InputHandler::doCommand(static_cast<KeyActions::KeyAction>('i'));
	EXPECT_EQ(Editor::mode(), Editor::Mode::EditMode);
}

TEST(InputTests, EnableReadModeWorks)
{
	InputHandler::handleInput(KeyActions::KeyAction::Esc);
	EXPECT_EQ(Editor::mode(), Editor::Mode::ReadMode);
}

TEST(InputTests, InsertCharChangesFile)
{
	bool beforeChange = Editor::isDirty();
	InputHandler::handleInput(static_cast<KeyActions::KeyAction>('c'));
	bool afterChange = Editor::isDirty();
	EXPECT_NE(beforeChange, afterChange);
}