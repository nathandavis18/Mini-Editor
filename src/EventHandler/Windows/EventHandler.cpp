#include "EventHandler/EventHandler.hpp"
#include "Editor/Editor.hpp"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include <thread>

HANDLE hStdIn;
std::thread t;
EventHandler::EventHandler(std::atomic<bool>& running) : mRunning(running)
{
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	t = std::thread(EventHandler::windowSizeChanged, std::ref(mRunning));
}

EventHandler::~EventHandler()
{
	while (!t.joinable());
	t.join();
}


void EventHandler::windowSizeChanged(std::atomic<bool>& running)
{
	while (running)
	{
		INPUT_RECORD input;
		DWORD numEvents;

		ReadConsoleInput(hStdIn, &input, 1, &numEvents); //Blocks this thread until an event happens
		if (input.EventType == WINDOW_BUFFER_SIZE_EVENT) //If the event is a window size update
		{
			Editor::updateWindowSize();
			Editor::refreshScreen(true);
		}
	}
}