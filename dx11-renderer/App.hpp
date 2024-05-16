#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include <memory>
#include "ImguiManager.hpp"

class App {
private:
	ImguiManager imgui;
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