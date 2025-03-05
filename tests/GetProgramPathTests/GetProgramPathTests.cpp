#include <gtest/gtest.h>
#include <filesystem>

#include "Utility/GetProgramPath/GetProgramPath.hpp"

TEST(GetProgramPathTests, ProgramPathReturnsSuccessfully)
{
	std::filesystem::path curPath = std::filesystem::current_path();
	std::filesystem::path returnedPath = GetProgramPath::getPath();

	EXPECT_EQ(curPath, returnedPath);
}