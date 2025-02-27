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
	struct JsonValue; //Forward declaration so JsonObject can access it

	//Creating some typedefs for the Json Objects
	using JsonObject = std::unordered_map<std::string, JsonValue>;
	using JsonSet = std::unordered_set<std::string>;
	using JsonValue_t = std::variant<std::string, JsonSet, JsonObject>;

	/// <summary>
	/// How the parsed Json data is stored and accessed
	/// </summary>
	struct JsonValue
	{
		JsonValue_t value; //A variant with a string value, an unordered_set value, or a new JsonObject
		
		/// <summary>
		/// Gets the JsonObject from the variant and checks if it contains a key
		/// </summary>
		/// <param name="key"></param>
		/// <returns></returns>
		const bool contains(const std::string& key) const
		{
			const JsonObject& obj = std::get<JsonObject>(value);
			return obj.contains(key);
		}

		/// <summary>
		/// Gets the sub object from the current object with the specified key
		/// </summary>
		/// <param name="key"></param>
		/// <returns></returns>
		const JsonValue& at(const std::string& key) const
		{
			const JsonObject& x = std::get<JsonObject>(value);
			return x.at(key);
		}

		/// <summary>
		/// A templated function for returning the value at a specific key
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="key"></param>
		/// <returns></returns>
		template <class T>
		const T& get(const std::string& key) const
		{
			const JsonValue& x = at(key);
			return std::get<T>(x.value);
		}

		bool operator==(const JsonValue& other) const
		{
			return value == other.value;
		}
	};

	/// <summary>
	/// The entry function into parsing the JSON file contents. Returns a vector of top-level keys
	/// </summary>
	/// <param name="contents"></param>
	/// <returns></returns>
	std::vector<JsonObject> parseJson(std::string_view contents);
}