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
#include "JsonParser.hpp"
namespace JsonParser
{
	using iter = std::string_view::iterator;

	JsonObject startParsing(std::string_view, iter&, bool initial = false);
	JsonSet findEndArray(std::string_view, iter&);
	const std::pair<std::string, JsonValue> getKeyValuePair(std::string_view contents, iter& currentPos, bool initial = false);

	JsonObject startParsing(std::string_view contents, iter& currentPos, bool initial)
	{
		JsonObject map;
		++currentPos;
		do
		{
			map.emplace(getKeyValuePair(contents, currentPos, initial));
			while ((*currentPos == ' ' || *currentPos == '\n') || (*currentPos != '\"' && *currentPos != '}')) ++currentPos;
		} while (*currentPos != '}');
		return map;
	}

	JsonSet findEndArray(std::string_view contents, iter& currentPos)
	{
		++currentPos;
		JsonSet allStrings;
		iter valueStartIter;
		do
		{
			while (*currentPos == ' ' || *currentPos == '\n' || *currentPos == ',') ++currentPos;
			if (*currentPos == '\"')
			{
				++currentPos;
				valueStartIter = currentPos;
				while (*currentPos != '\"') ++currentPos;
				allStrings.insert(contents.substr(valueStartIter - contents.begin(), currentPos - valueStartIter));
				++currentPos;
			}
		} while (*currentPos != ']');
		return allStrings;
	}

	const std::pair<std::string, JsonValue> getKeyValuePair(std::string_view contents, iter& currentPos, bool initial)
	{
		while (*currentPos == ' ' || *currentPos == '\n' || *currentPos == ',')
		{
			++currentPos;
		}

		std::string key;
		iter keyValueIter;
		JsonValue value;
		if (*currentPos == '\"')
		{
			++currentPos;
			keyValueIter = currentPos;
			while (*currentPos != '\"')
			{
				++currentPos;
			}
			key = contents.substr(keyValueIter - contents.begin(), currentPos - keyValueIter); //Found key
		}

		if (key == std::string()) key = "null";

		while (*currentPos != ':' && *currentPos != '}') ++currentPos;
		if(*currentPos != '}') ++currentPos;
		while (*currentPos == ' ' || *currentPos == '\n' || *currentPos == ',') ++currentPos;

		if (*currentPos == '{')
		{
			std::string_view newContents = contents.substr(currentPos - contents.begin());
			iter newStart = newContents.begin();
			value = { startParsing(newContents, newStart) };
			currentPos += newStart - newContents.begin();
		}
		else if(*currentPos == '[')
		{
			value = { findEndArray(contents, currentPos) };
		}
		else if (*currentPos == '\"')
		{
			++currentPos;
			keyValueIter = currentPos;
			while (*currentPos != '\"') ++currentPos;
			value = { std::string(contents.substr(keyValueIter - contents.begin(), currentPos - keyValueIter)) };
		}
		else
		{
			value = { std::string("null") };
		}
		if(!initial) ++currentPos;

		return std::make_pair(key, value);
	}

	std::vector<JsonObject> parseJson(std::string_view contents)
	{
		if (contents.empty() || contents.at(0) != '{') return std::vector<JsonObject>();

		iter currentPos = contents.begin();
		std::vector<JsonObject> vec;
		do
		{
			contents = contents.substr(currentPos - contents.begin());
			currentPos = contents.begin();
			JsonObject obj = startParsing(contents, currentPos, true);
			vec.push_back(obj);
			if((currentPos - contents.begin()) != contents.find_last_of('}')) ++currentPos;
			while (*currentPos != '}' && *(currentPos + 1) != '\"') ++currentPos;
		} while (*currentPos != '}');

		return vec;
	}
}