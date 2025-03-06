#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <sstream>

#include "Utility/JsonParser/JsonParser.hpp"

using JsonParser::JsonObject;
using JsonParser::JsonSet;
using JsonParser::JsonValue;

TEST(JsonParserTests, JsonGetsParsedSuccessfully)
{
	std::vector<JsonObject> obj;
	JsonSet set = { "item1", "item2" };
	JsonObject innerObj;
	innerObj.emplace(std::make_pair("something", "value"));

	JsonObject objPartial;
	objPartial.emplace(std::make_pair("test2", set));
	objPartial.emplace(std::make_pair("test3", innerObj));

	JsonObject fullObj;
	fullObj.emplace(std::make_pair("test", objPartial));

	obj.push_back(fullObj);

	std::ifstream f("testJson.json");
	std::stringstream stream;
	stream << f.rdbuf();

	std::vector<JsonObject> obj2 = JsonParser::parseJson(stream.str());
	EXPECT_EQ(obj, obj2);
}