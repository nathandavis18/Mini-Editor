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
#include "FindAndReplace.hpp"

namespace FindAndReplace
{
	std::vector<FindLocation> find(const std::string_view strToFind, const std::vector<FileHandler::Row>& fileRows)
	{
		std::vector<FindLocation> findLocations;

		for (size_t i = 0; i < fileRows.size(); ++i)
		{
			const std::string_view line = fileRows.at(i).line;
			size_t findPos;
			size_t offset = 0;
			while ((findPos = line.find(strToFind, offset)) != std::string_view::npos)
			{
				findLocations.emplace_back(i, findPos, strToFind.length(), findPos);
				offset = findPos + strToFind.length();
			}
		}

		return findLocations;
	}

	void replace(std::string& line, const std::string& insertStr, const FindLocation location)
	{
		line.erase(location.startCol, location.length);
		line.insert(location.startCol, insertStr);
	}
}