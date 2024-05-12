#pragma once
#include "Window.hpp"
#include "Timer.hpp"
class App {
private:
	Window wnd;
	Timer timer;
	bool keepRunning = true;
public:
	App();
	int Go();
private:
	void DoFrame();
};