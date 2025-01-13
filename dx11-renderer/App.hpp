#pragma once
#include "Window.hpp"
#include "Timer.hpp"
#include <memory>
#include "ImguiManager.hpp"
#include "Camera.hpp"
#include "FirstPersonCamera.hpp"
#include "PointLight.hpp"
#include "Mesh.hpp"
#include "TestPlane.hpp"
#include "TaskManager.hpp"

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
	bool canToggle = true;
	float speed_factor = 1.0f;
	float prevMouseX = 0;
	float prevMouseY = 0;
	DirectX::XMMATRIX projection;
	IntersectionResult result;
	DirectX::XMFLOAT3 roFloat{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 rdFloat{ 0.0f, 0.0f, 0.0f };
	int x = 0;
	int y = 0;
	std::string _commandLine;
public:
	App(std::string commandLine = "");
	int Go();
	~App() 
	{
		TaskManager::Get().Stop();
		TaskManager::Get().Wait();
	}
private:
	void DoFrame();
	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 4;
	std::unique_ptr<Model> model;
	std::unique_ptr<Model> pokeWall;
	std::unique_ptr<Model> gobber;
	std::unique_ptr<Model> sponza;
	std::unique_ptr<TestPlane> plane;

};