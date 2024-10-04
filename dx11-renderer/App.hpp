#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include <memory>
#include "ImguiManager.hpp"
#include "Camera.hpp"
#include "FirstPersonCamera.hpp"
#include "PointLight.hpp"

class App {
private:
	ImguiManager imgui;
	Window wnd;
	Timer timer;
	Camera cam;
	FirstPersonCamera _fpsCam;
	PointLight light;
	bool keepRunning = true;
	bool _showCursor = true;
	float speed_factor = 1.0f;
	float prevMouseX = 392;
	float prevMouseY = 269;
public:
	App();
	int Go();
	~App() {}
private:
	void DoFrame();
	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 180;
};