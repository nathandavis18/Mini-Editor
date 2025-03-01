#pragma once
#include <functional>
#include <atomic>
#include <cstdint>
class EventHandler
{
public:
	EventHandler(std::atomic<bool>& running);
	~EventHandler();
	static void windowSizeChanged(std::atomic<bool>& running);

private:
	std::atomic<bool>& mRunning;
};