/**
* MIT License

Copyright (c) 2024 Nathan Davis

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

FileHandler::FileHandler(const std::string_view fName) : mPath(std::filesystem::current_path() / fName), mFileName(fName)
{
	loadFileContents();
}

void FileHandler::loadRows(std::string&& str)
{
	if (str.length() > 0)
	{
		size_t lineBreak = 0;
		while (lineBreak < str.length())
		{
			if (str.at(lineBreak) == '\n')
			{
				if (lineBreak > 0 && str.at(lineBreak - 1) == '\r')
				{
					mRows.emplace_back(str.substr(0, lineBreak - 1)); //Remove the carriage return as well
				}
				else
				{
					mRows.emplace_back(str.substr(0, lineBreak));
				}
				str.erase(str.begin(), str.begin() + lineBreak + 1);
				lineBreak = 0;
			}
			else
			{
				++lineBreak;
			}
		}
		mRows.emplace_back(str);
		str.clear();
	}
}

const std::string_view FileHandler::fileName()
{
	return mFileName;
}

void FileHandler::loadFileContents()
{
	std::ifstream file(mPath);
	try
	{
		std::stringstream ss;
		ss << file.rdbuf();

		loadRows(std::move(ss.str()));
	}
	catch (std::exception ex)
	{
		file.close();
		std::cerr << "Error opening file. ERROR: " << ex.what() << std::endl;
		exit(EXIT_FAILURE);
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