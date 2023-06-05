#pragma once
#include "Window.hpp"
#include "Timer.hpp"
class App {
private:
	Window wnd;
	Timer timer;
public:
	App();
	int Go();
private:
	void DoFrame();
};