/**
* MIT License

Copyright (c) 2025 Nathan Davis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Renderer.hpp"

#include <format>
#include <iostream>

constexpr char MiniVersion[7] = "0.8.0a";

Renderer::Renderer() : mTextRenderBuffer("\x1b[H") {}

void Renderer::addRenderedLineToBuffer(const std::string& renderedLine)
{
	mTextRenderBuffer.append(renderedLine);
	mTextRenderBuffer.append("\x1b[0K\r\n");
}

void Renderer::addEndOfFileToBuffer(const uint16_t rowsToEnter, const uint16_t colCount, const bool emptyFile)
{
	constexpr char emptyRowCharacter[2] = "~";
	mTextRenderBuffer.append("\x1b[0m");
	for (uint16_t i = 1; i <= rowsToEnter; ++i)
	{
		if (emptyFile && i == rowsToEnter / 3)
		{
			const std::string emptyFileMessage = std::format("Mini Editor -- version {}\x1b[0K\r\n", MiniVersion);
			uint16_t padding = (colCount - emptyFileMessage.length()) / 2;
			if (padding > 0)
			{
				mTextRenderBuffer.append(emptyRowCharacter);
				--padding;

				mTextRenderBuffer.insert(mTextRenderBuffer.length(), padding, ' ');
			}
			mTextRenderBuffer.append(emptyFileMessage);
		}
		else
		{
			mTextRenderBuffer.append(emptyRowCharacter);
			mTextRenderBuffer.append("\x1b[0K\r\n");
		}
	}
}

void Renderer::renderScreen(const bool forceDraw, const bool renderCommandBuffer)
{
	std::string bufferToRender;
	mTextRenderBuffer.append("\x1b[3J");

	if (mTextRenderBuffer != mPreviousTextRenderBuffer || forceDraw)
	{
		bufferToRender.append(mTextRenderBuffer);
		mPreviousTextRenderBuffer = mTextRenderBuffer;
	}
	bufferToRender.append(mStatusBuffer);
	bufferToRender.append(mCursorBuffer);

	if (renderCommandBuffer) bufferToRender.append(mCommandBuffer);

	std::cout << bufferToRender;
	std::cout.flush();

	mTextRenderBuffer = "\x1b[H";
}

void Renderer::setStatusBuffer(const uint16_t statusRowStart, const bool dirty, const std::string_view fileName, const size_t numRows, const size_t currentRow, const size_t currentCol, const std::string& mode, const std::string& rStatus, const size_t maxLength)
{
	constexpr uint8_t statusColStart = 0;
	mStatusBuffer = std::format("\x1b[{};{}H", statusRowStart, statusColStart);
	mStatusBuffer.append("\x1b[0m\x1b[0K\x1b[7m");
	
	std::string fileInfo = std::format("{} - {} lines {}", fileName, numRows, dirty ? "(modified)" : "");

	mStatusBuffer.append(fileInfo);

	size_t currentStatusLength = fileInfo.length();

	while ((maxLength / 2) - currentStatusLength > mode.length() / 2)
	{
		mStatusBuffer.append(" ");
		++currentStatusLength;
	}

	mStatusBuffer.append(mode);
	currentStatusLength += mode.length();

	while (currentStatusLength + rStatus.length() < maxLength)
	{
		mStatusBuffer.append(" ");
		++currentStatusLength;
	}
	mStatusBuffer.append(rStatus);
	mStatusBuffer.append("\r\n\x1b[0K");

	mStatusBuffer.append("\x1b[0m");
}

void Renderer::setCursorBuffer(const uint16_t cursorRow, const uint16_t cursorCol)
{
	mCursorBuffer = std::format("\x1b[{};{}H", cursorRow, cursorCol);
}

void Renderer::setCommandBuffer(const std::string& commandBuffer, const size_t commandBufferRow)
{
	constexpr uint8_t statusColStart = 0;
	mCommandBuffer = std::format("\x1b[{};{}H", commandBufferRow, statusColStart);
	mCommandBuffer.append("\x1b[0K");
	mCommandBuffer.append(commandBuffer);
}

void Renderer::clearScreen()
{
	std::string clearScreen = "\x1b[2J\x1b[3J\x1b[H"; //Clear screen, clear saved lines, and move cursor to Home (0,0)
	std::cout << clearScreen;
	std::cout.flush();
}