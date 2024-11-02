#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include <memory>
#include "ImguiManager.hpp"
#include "Camera.hpp"
#include "FirstPersonCamera.hpp"
#include "PointLight.hpp"
#include "Mesh.hpp"

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
	float prevMouseX = 0;
	float prevMouseY = 0;
	DirectX::XMMATRIX projection;
	IntersectionResult result;
	DirectX::XMFLOAT3 roFloat;
	DirectX::XMFLOAT3 rdFloat;
	int x = 0;
	int y = 0;
public:
	App();
	int Go();
	~App() {}
private:
	void DoFrame();
	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 4;
	std::unique_ptr<Model> model;
};