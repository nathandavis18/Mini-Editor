#include "FindString.hpp"

namespace FindString
{
	std::vector<FindLocation> FindString::find(const std::string_view strToFind, const std::vector<FileHandler::Row>& fileRows)
	{
		std::vector<FindLocation> findLocations;

		for (size_t i = 0; i < fileRows.size(); ++i)
		{
			const std::string_view line = fileRows.at(i).line;
			size_t findPos;
			size_t offset = 0;
			while ((findPos = line.find(strToFind, offset)) != std::string_view::npos)
			{
				findLocations.emplace_back(i, findPos, strToFind.length());
				offset = findPos + strToFind.length();
			}
		}

		return findLocations;
	}
}