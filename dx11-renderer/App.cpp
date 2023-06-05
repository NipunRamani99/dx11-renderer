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
	float time = timer.Peek().count();
	std::ostringstream oss;
	oss << "Time Elapsed: " << std::setprecision(2) << time;
	wnd.SetTitle(oss.str());
}
