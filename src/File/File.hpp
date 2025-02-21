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
#include <string>
#include <string_view>
#include <vector>

namespace FileHandler
{
	namespace detail
	{
		/// <summary>
		/// Splits the file contents into rows for the editor to use
		/// Called by loadFileContents()
		/// </summary>
		/// <param name=""></param>
		void loadRows(std::string&&);

		/// <summary>
		/// Loads the file into a stringstream for loadRows to work with.
		/// Called by initFileHandler()
		/// </summary>
		void loadFileContents();
	}
	/// <summary>
	/// The structure of each row in the file.
	/// line is what is actually stored, including \t and other characters
	/// renderedLine is what gets displayed to the user
	/// </summary>
	struct Row
	{
		std::string line;
		std::string renderedLine;
	};
	
	/// <summary>
	/// Called during Editor Initialization. Initializes all the internals of the file handler, including file name and path.
	/// </summary>
	/// <param name="fName"></param>
	void initFileHandler(const std::string_view fName);

	/// <summary>
	/// Returns the file name for the editor to display
	/// Called by the editor on every status bar refresh
	/// </summary>
	/// <returns></returns>
	const std::string_view fileName();

	/// <summary>
	/// Moves the file contents to the editor since the file handler no longer needs it. 
	/// </summary>
	/// <returns></returns>
	std::vector<Row>&& getFileContents();

	/// <summary>
	/// Saves the current contents to the file
	/// Called when a save or save/quit command is used.
	/// </summary>
	/// <param name="newContents"></param>
	void saveFile(const std::string_view newContents);

}