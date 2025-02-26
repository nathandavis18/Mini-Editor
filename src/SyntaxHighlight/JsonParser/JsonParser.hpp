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
#include <array>
#include <string>
#include <string_view>

namespace JsonParser
{
	struct JsonValue;

	using JsonObject = std::unordered_map<std::string, JsonValue>;
	using JsonValue_t = std::variant<std::string, std::unordered_set<std::string_view>, JsonObject>;
	using JsonSet = std::unordered_set<std::string_view>;

	struct JsonValue
	{
		JsonValue_t value;
		
		const bool contains(const std::string& key) const
		{
			const JsonObject& obj = std::get<JsonObject>(value);
			return obj.contains(key);
		}
		const JsonValue& at(const std::string& key) const
		{
			const JsonObject& x = std::get<JsonObject>(value);
			return x.at(key);
		}
		operator const JsonValue_t& () const
		{
			return value;
		}

		template <class T>
		const T& get(const std::string& key) const
		{
			const JsonValue& x = at(key);
			return std::get<T>(x.value);
		}
	};


	std::vector<JsonObject> parseJson(std::string_view contents);
}