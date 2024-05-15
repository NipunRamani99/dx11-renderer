#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include <memory>


class App {
private:
	Window wnd;
	Timer timer;
	bool keepRunning = true;
public:
	App();
	int Go();
	~App() {}
private:
	void DoFrame();
	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 180;
};