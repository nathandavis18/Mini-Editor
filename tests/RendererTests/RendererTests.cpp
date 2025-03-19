#include <gtest/gtest.h>
#include <string>
#include <format>

#include "Renderer/Renderer.hpp"

TEST(RendererTests, RendererWritesToConsole)
{
	Renderer renderer;

	testing::internal::CaptureStdout();
	renderer.renderScreen(true, false);

	EXPECT_NE(testing::internal::GetCapturedStdout(), std::string());
}

TEST(RendererTests, RendererAddsLineToBuffer)
{
	Renderer renderer;

	std::string testLine = "This is a test rendered line";
	renderer.addRenderedLineToBuffer(testLine);
	testing::internal::CaptureStdout();
	renderer.renderScreen(true, false);
	std::string output = testing::internal::GetCapturedStdout();

	bool outputContainsLine = (output.find(testLine) != std::string::npos);
	EXPECT_TRUE(outputContainsLine);
}

TEST(RendererTests, RendererAddsStatusToBuffer)
{
	Renderer renderer;

	std::string mode = "MODE";

	std::string rStatus = "Right-end of status";

	renderer.setStatusBuffer(1, false, "testFile.txt", 1, 1, 1, mode, rStatus, 120);
	testing::internal::CaptureStdout();
	renderer.renderScreen(false, false);
	std::string output = testing::internal::GetCapturedStdout();

	bool outputContainsStatus = ((output.find(mode) != std::string::npos) && (output.find(rStatus) != std::string::npos));
	EXPECT_TRUE(outputContainsStatus);
}

TEST(RendererTests, RendererSetsCursorPosition)
{
	Renderer renderer;

	size_t cursorRow = 10, cursorCol = 15;
	std::string expectedString = std::format("\x1b[{};{}H", cursorRow, cursorCol);

	renderer.setCursorBuffer(cursorRow, cursorCol);

	testing::internal::CaptureStdout();
	renderer.renderScreen(false, false);
	std::string output = testing::internal::GetCapturedStdout();

	bool cursorSetProperly = (output.find(expectedString) != std::string::npos);
	EXPECT_TRUE(cursorSetProperly);
}

TEST(RendererTests, RendererRendersCommandBuffer)
{
	Renderer renderer;

	std::string commandBuffer = "Test Command Buffer";

	renderer.setCommandBuffer(commandBuffer, 1);

	testing::internal::CaptureStdout();
	renderer.renderScreen(false, true);
	std::string output = testing::internal::GetCapturedStdout();

	bool commandBufferRendered = (output.find(commandBuffer) != std::string::npos);
	EXPECT_TRUE(commandBufferRendered);
}

TEST(RendererTests, NoUnnecessaryRerendersOfMainText)
{
	Renderer renderer;

	renderer.addRenderedLineToBuffer("Test Line");

	testing::internal::CaptureStdout();
	renderer.renderScreen(false, false);
	std::string output = testing::internal::GetCapturedStdout();

	renderer.addRenderedLineToBuffer("Test Line");

	testing::internal::CaptureStdout();
	renderer.renderScreen(false, false);
	std::string output2 = testing::internal::GetCapturedStdout(); //Shouldn't contain main text buffer since it is just a re-render

	EXPECT_NE(output, output2);
}

TEST(RendererTests, ForceRerenderWorks)
{
	Renderer renderer;

	renderer.addRenderedLineToBuffer("Test Line");

	testing::internal::CaptureStdout();
	renderer.renderScreen(false, false);
	std::string output = testing::internal::GetCapturedStdout();

	renderer.addRenderedLineToBuffer("Test Line");

	testing::internal::CaptureStdout();
	renderer.renderScreen(true, false);
	std::string output2 = testing::internal::GetCapturedStdout(); //Shouldn't contain main text buffer since it is just a re-render

	EXPECT_EQ(output, output2);
}