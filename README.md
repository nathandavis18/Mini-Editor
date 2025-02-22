# NotVim Editor

NotVim Editor is a WIP, custom, 0 dependencies, cross-platform, console-based text editor, currently tested to work on Windows and Linux.

All standard features of a text-editor, with the exception of find/replace, have been implemented and should be bug-free. 
Be sure to check out [Getting Starting](#getting-started)!
<hr>

### Demo


https://github.com/user-attachments/assets/ba6fbc34-5d92-448f-b9ac-2994f407eeba


<hr>

### Versions
If you would like to try the latest features, you can build the application from the dev branch. However, some features may be experimental, and bugs may arise.

If you would like to use a stable release, check out the releases tab. Releases are tested to be as stable as possible. Some bugs may slip through to releases, so be sure to report those!

<hr>

### Controls

Controls are listed as follows:

	WHILE IN READ MODE (Default Mode):
	- i - Enable Edit Mode
	- : - Enable Command Mode
	
	WHILE IN COMMAND MODE:
	- q: Quit (File must be saved if changes have been made)
	- q!: Force Quit. Don't even check if file has been saved
	- w/s: [W]rite/[S]ave changes
	- wq/sq: [W]rite and [Q]uit / [S]ave and [Q]uit.

	WHILE IN EDIT MODE:
	Escape: Go back to Read Mode

	MOVEMENT FUNCTIONALITY:
	- ArrowKey Left/Right: Move 1 character left/right within the file.
	- ArrowKey Up/Down: Move one row up/down within the file
	- CtrlArrow Left/Right: Move to the start/end of the previous word/next word
	- CtrlArrow Up/Down: Shift the current view offset by one up/down
	- Home/End: Go to start of/end of current row
	- CtrlHome/End: Go to start of / end of file
	- PageUp/Down: Shift screen position by 1 page/screen rows worth
	- CtrlPage Up/Down: Move cursor to start of/end of screen without adjusting screen position

	CHARACTER EDITING:
	- Letter/Number/Symbol: Insert at current cursor position and move cursor forward
 	- Tab: Insert a tab character, equivalent to cursor % 8 spaces. 
	- Enter/Return: Insert a new row, moving contents beyond the cursor onto the new row and move cursor to start of new row
	- Backspace/Delete: Delete character behind/in front of cursor. Move cursor backwards if using backspace.
	- CtrlBackspace/Delete: Delete word
 	- Ctrl+Z: Undo most recent change.
  	- Ctrl+Y: Redo most recent undo.

More key functionality will be added as this project progresses.

<hr>

### Syntax Highlighting

Currently syntax highlighting only supports C++ files (.cc, .cpp, .hpp, .h, .hh, etc.). This is currently hard-coded into the syntax highlight, but a config file will be implemented in the near future!

Syntax highlights are generated using a custom token system, which stores the highlight type, starting position, and ending position. For multiline highlights, including strings and multiline comments,
the token also stores if the ending position exists, or if the rest of the file should just have the same highlight.

Although the C++ values are hard-coded into the syntax highlight system, it is designed to be fully dynamic. All you need to do is add the keywords and comment syntax to the syntax highlight vector.
You can use the current system as a base to see how it works. Then you can also define some colors using the link provided at the top of SyntaxHighlight.cpp.

![image](https://github.com/user-attachments/assets/c1a5efd0-996f-4f5f-8dae-d87293c9a905)

<hr>

### Getting Started

If you are on Windows and would like to try this out right away, head to the releases tab and download the nve.exe file. This is the standalone program executable.
Check out [Usage](#usage) to learn how to use it!

Be sure to check out [Building](#building) if you would like to build the project for yourself!

If you would like to contribute, check out [Contributing](#contributing)!

<hr>

### Building

A CMake Build script is provided. CMake 3.12, as well as a C++20 compliant compiler is required. A python script to automate the cmake build process is also provided, if you have Python installed.

To build with the Python script, run

	python build.py

This will build the project into the folder path ./out/build

To build this project with the tests, run 

	python build.py --build-tests

To build manually, run

	cmake -B {buildDir} -G {buildGenerator} -S {pathToCMakeScript} -DCMAKE_BUILD_TYPE=Release

	cmake --build {buildDir} --config Release

	EXAMPLE (While in root dir):

	cmake -B ./out -G Ninja -DCMAKE_BUILD_TYPE=Release

	cmake --build ./out --config Release

To build manually with tests, run the following instead

	cmake -B {buildDir} -G {buildGenerator} -S {pathToCMakeScript} -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
	
	EXAMPLE:
	
	cmake -B ./out -G Ninja _DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
	
	cmake--build ./out --config Release

<hr>

### Usage

To use, navigate to the executable (either in {buildDir} or {buildDir}/bin most commonly). Then, run

	./nve <filename.fileExtension>

	OR IF USING COMMAND PROMPT

	nve <filename.fileExtension>

	EXAMPLE:

	./nve test.cpp

This executable is a standalone executable, so you may also add this file to your system path and use it from anywhere

To run the tests, navigate to the Tests executable (located in {buildDir}/tests, or {buildDir}/tests/Release).

Run it by doing

	./Tests

	OR IF USING COMMAND PROMPT

	Tests

<hr>

### Known Bugs

Check the issues page to see all known bugs

<hr>

### Contributing

I am welcome to any and all who want to contribute! Pushing to the master branch is restricted (for obvious reasons, lol). If you would like to contribute, 
just clone the repo, make a new branch, and implement whatever features you would like! You can also fork this repo and contribute that way. All changes must be approved by me, 
as I am the sole maintainer of this project. 

<hr>

#### UNTESTED ON OSX (macOS). 
 
From my understanding, this should still work on mac due to the Unix based nature of it, but it is untested.
