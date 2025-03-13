#pragma once

#include "File/File.hpp"

#include <vector>
#include <string_view>

namespace FindString
{
	struct FindLocation
	{
		size_t row = 0, startCol = 0, length = 0;
	};

	std::vector<FindLocation> find(const std::string_view strToFind, const std::vector<FileHandler::Row>& fileRows);
}