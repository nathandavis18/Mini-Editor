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
#include <filesystem>
#include <fstream>

namespace FileHandler
{
	std::string _fileName;
	std::filesystem::path _path;
	std::vector<Row> _fileContents;

	void detail::loadRows(std::string&& str)
	{
		if (str.length() > 0)
		{
			size_t lineBreak = 0;
			while ((lineBreak = str.find("\r\n")) != std::string::npos)
			{
				_fileContents.emplace_back(str.substr(0, lineBreak));
				str.erase(str.begin(), str.begin() + lineBreak + 2);
			}
			_fileContents.emplace_back(str);
			str.clear();
		}
	}

	void detail::loadFileContents()
	{
		std::ifstream file(_path);
		try
		{
			std::stringstream ss;
			ss << file.rdbuf();

			detail::loadRows(std::move(ss.str()));
		}
		catch (std::exception ex)
		{
			file.close();
			std::cerr << "Error opening file. ERROR: " << ex.what() << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	void initFileHandler(const std::string_view fName)
	{
		_fileName = fName;
		_path = std::filesystem::current_path() / _fileName;
		detail::loadFileContents();
	}

	const std::string_view fileName()
	{
		return _fileName;
	}


	std::vector<Row>&& getFileContents()
	{
		return std::move(_fileContents);
	}

	void saveFile(const std::string_view newContents)
	{
		std::ofstream file(_path);
		try
		{
			file << newContents;
		}
		catch (std::exception ex)
		{
			file.close();
			std::cerr << "Error saving file. ERROR: " << ex.what() << std::endl;
		}
	}
}