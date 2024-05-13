#include "App.hpp"
#include "Box.hpp"
App::App()
	:
	wnd(800,600,"DX11 Renderer")
{
	box = std::make_unique<Box>(wnd.Gfx());
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
}

int App::Go()
{
	while (keepRunning) {
		if (auto ecode = wnd.ProcessMessage()) {
			return *ecode;
		}
		DoFrame();
		wnd.CenterCursorPosition();
		if (wnd.kbd.KeyIsPressed(VK_ESCAPE)) {
			keepRunning = false;
		}
	}
	return 0;
}


void App::DoFrame()
{
	static float angle = 0.0f;
	const float c = sin(timer.Peek().count()) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer(c, c, 1.0f);
	float mouseX = 2.0f* wnd.mouse.GetPosX()/800.0f - 1;
	float mouseY = -2.0f*wnd.mouse.GetPosY()/600.0f + 1;
	auto dt = timer.Mark();
	/*wnd.Gfx().DrawTestTriangle(
		timer.Peek().count(),
		wnd.mouse.GetPosX() / 400.0f - 1.0f,
		-wnd.mouse.GetPosY() / 300.0f + 1.0f
	);*/
	box->Update(dt.count());
	box->Draw(wnd.Gfx());
	wnd.Gfx().EndFrame();
	angle += 0.001f;
}
