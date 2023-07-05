#include "App.hpp"

App::App()
	:
	wnd(640,480,"DX11 Renderer")
{
}

int App::Go()
{
	while (true) {
		if (auto ecode = wnd.ProcessMessage()) {
			return *ecode;
		}
		DoFrame();
	}
	return 0;
}

void App::DoFrame()
{
	static float angle = 0.0f;
	const float c = sin(timer.Peek().count()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	float mouseX = wnd.mouse.GetPosX();
	float mouseY = wnd.mouse.GetPosY();
	OutputDebugString(std::to_string(mouseX).c_str());
	OutputDebugString("\n");
	OutputDebugString(std::to_string(mouseY).c_str());
	wnd.Gfx().DrawTestTriangle(angle,
		mouseX,
		mouseY);
	wnd.Gfx().EndFrame();
	angle += 0.001f;
}
