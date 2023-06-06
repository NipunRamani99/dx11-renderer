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
	const float c = sin(timer.Peek().count()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	wnd.Gfx().EndFrame();
}
