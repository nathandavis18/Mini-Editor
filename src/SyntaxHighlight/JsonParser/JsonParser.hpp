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
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
#include <string>
#include <string_view>

namespace JsonParser
{
	struct JsonValue;

	using JsonSomething = std::variant<std::nullptr_t, std::string, std::unordered_set<std::string>, std::unordered_map<std::string, JsonValue>>;

	struct JsonValue
	{
		JsonSomething value;
		const JsonValue& at(const std::string& key) const
		{
			const std::unordered_map<std::string, JsonValue>& x = std::get<std::unordered_map<std::string, JsonValue>>(value);
			return x.at(key);
		}
		const JsonValue& operator[](const std::string& key) const
		{
			const std::unordered_map<std::string, JsonValue>& x = std::get<std::unordered_map<std::string, JsonValue>>(value);
			return x.at(key);
		}
		const std::string& getValue() const
		{
			const std::string& finalValue = std::get<std::string>(value);
			return finalValue;
		}
	};
	using JsonObject = std::unordered_map<std::string, JsonValue>;
	using iter = std::string_view::iterator;


	std::vector<JsonObject> parseJson(std::string_view contents);
}