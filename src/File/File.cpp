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

#include "File.hpp"

#include <exception>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>

unsigned int maxThreads;
constexpr uint16_t maxLengthPerThread = 20'000;
constexpr uint8_t charactersPerRowAverage = 20; //Assume an average of 20 characters per row. This will need some testing to fine-tune

FileHandler::FileHandler(const std::string_view fName) : mPath(std::filesystem::current_path() / fName), mFileName(fName)
{
	maxThreads = std::thread::hardware_concurrency();
	loadFileContents();
}

const std::string_view FileHandler::fileName()
{
	return mFileName;
}

void FileHandler::loadRows(const size_t startPos, const size_t endPos, const std::string_view str, std::promise<std::vector<FileHandler::Row>>&& p)
{
	std::vector<FileHandler::Row> rows;
	rows.reserve((endPos - startPos) / charactersPerRowAverage);
	size_t findPos;
	std::string_view threadStr = str.substr(startPos, endPos - startPos);

	while ((findPos = threadStr.find('\n')) != std::string_view::npos)
	{
		if (findPos > 0 && threadStr.at(findPos - 1) == '\r')
		{
			rows.emplace_back(std::string(threadStr.substr(0, findPos - 1)));
		}
		else
		{
			rows.emplace_back(std::string(threadStr.substr(0, findPos)));
		}
		threadStr = threadStr.substr(findPos + 1);
	}
	rows.emplace_back(std::string(threadStr));
	p.set_value(rows);
}


void FileHandler::loadFileContents()
{
	std::ifstream file(mPath);
	std::stringstream fileContents;
	fileContents << file.rdbuf();
	file.close();

	const std::string& fileStr = fileContents.str();
	if (fileStr.length() == 0) return;

	if (fileStr.length() <= maxLengthPerThread)
	{
		std::promise<std::vector<FileHandler::Row>> promise;
		std::future<std::vector<FileHandler::Row>> value = promise.get_future();
		loadRows(0, fileStr.length(), fileStr, std::move(promise));
		mRows = value.get();
		return;
	}

	std::vector<std::thread> allThreads;
	std::vector<std::future<std::vector<FileHandler::Row>>> retValues;
	unsigned int threads = (fileStr.length() / maxLengthPerThread) + 1;
	if (threads > maxThreads) threads = maxThreads;
	const uint16_t lengthPerThread =  maxLengthPerThread / threads; //splitting the work up evenly
	size_t prevEndPos = 0;
	for (unsigned int i = 0; i < threads; ++i)
	{
		size_t startPos = prevEndPos;
		if (i > 0) ++startPos;

		size_t endPos = (i == threads - 1) ? fileStr.length() : startPos + lengthPerThread;

		if (endPos < fileStr.length())
		{
			while (fileStr.at(endPos) != '\n' && endPos != fileStr.length()) ++endPos;
		}
		prevEndPos = endPos;

		std::promise<std::vector<FileHandler::Row>> p;
		retValues.emplace_back(p.get_future());
		allThreads.emplace_back(&FileHandler::loadRows, this, startPos, endPos, fileStr, std::move(p));

		if (endPos == fileStr.length()) break;
	}

	mRows.reserve(fileStr.length() / charactersPerRowAverage);

	for (unsigned int i = 0; i < threads; ++i)
	{
		allThreads.at(i).join();
		auto& value = retValues.at(i);
		const std::vector<FileHandler::Row>& threadFileRows = value.get();

		mRows.insert(mRows.end(), threadFileRows.begin(), threadFileRows.end());
	}
}

std::vector<FileHandler::Row>* FileHandler::getFileContents()
{
	return &mRows;
}

void FileHandler::saveFile()
{
	std::stringstream output;
	for (size_t i = 0; i < mRows.size(); ++i)
	{
		if (i == mRows.size() - 1)
		{
			output << mRows.at(i).line;
		}
		else [[ likely ]]
		{
			output << mRows.at(i).line << '\n';
		}
	}
	std::ofstream file(mPath);

	try
	{
		file << output.str();
	}
	catch (std::exception ex)
	{
		file.close();
		std::cerr << "Error saving file. ERROR: " << ex.what() << std::endl;
	}
}