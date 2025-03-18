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
#pragma once

#include "File/File.hpp"

#include <vector>
#include <string_view>
#include <string>

namespace FindAndReplace
{
	/// <summary>
	/// The structure for storing the find locations.
	/// </summary>
	struct FindLocation
	{
		size_t row = 0, startCol = 0, length = 0, filePos = 0;
	};

	/// <summary>
	/// Finds all the strings that match a given string and builds the location vector. Returns the vector after all locations are found.
	/// Currently this is a blocking call, so on large files this may cause performance issues
	/// </summary>
	/// <param name="strToFind"></param>
	/// <param name="fileRows"></param>
	/// <returns></returns>
	std::vector<FindLocation> find(const std::string_view strToFind, const std::vector<FileHandler::Row>& fileRows);

	void replace(std::string& line, const std::string& insertStr, const FindLocation location);
}