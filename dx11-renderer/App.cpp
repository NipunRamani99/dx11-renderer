#include "App.hpp"
#include "Box.hpp"
#include "Melon.hpp"
#include "Pyramid.hpp"
#include "Cone.hpp"
#include <algorithm>
#include "ChiliMath.hpp"
#include "GDIPlusManager.hpp"
#include "Surface.hpp"
#include "Sheet.hpp"
#include "SkinnedBox.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

GDIPlusManager gdipm;
void GeometryAssortmentScene(Graphics& gfx, std::vector<std::unique_ptr<Drawable>>& drawables, size_t nDrawables) {
	class DrawableFactory {
	private:
		Graphics& gfx;
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_int_distribution<int> latdist{ 5,20 };
		std::uniform_int_distribution<int> longdist{ 10,40 };
		std::uniform_int_distribution<int> typedist{ 0, 4 };
	public:
		DrawableFactory(Graphics& gfx)
			:
			gfx(gfx)
		{
		}

		std::unique_ptr<Drawable> operator()() {
			switch (typedist(rng)) {
			case 0:
				return std::make_unique<Pyramid>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
				break;
			case 1:
				return std::make_unique<Box>(
					gfx, rng, adist, ddist,
					odist, rdist, bdist
				);
				break;
			case 2:
				return std::make_unique<Melon>(
					gfx, rng, adist, ddist,
					odist, rdist, longdist, latdist
				);
				break;
			case 3:
				return std::make_unique<Sheet>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			case 4:
				return std::make_unique<SkinnedBox>(
					gfx, rng, adist, ddist,
					odist, rdist
				);
			default:
				assert(false && "bad drawable type in factory");
				return {};
			}
		}

	};
	DrawableFactory factory(gfx);
	//box = std::make_unique<Box>(wnd.Gfx());
	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), nDrawables, factory);

}

App::App()
	:
	imgui(),
	wnd(800, 600, "DX11 Renderer")
{
	GeometryAssortmentScene(wnd.Gfx(), drawables, nDrawables);
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
	//box->Update(dt.count());
	//box->Draw(wnd.Gfx());
	for (auto& d : drawables)
	{
		d->Update(wnd.kbd.KeyIsPressed(VK_SPACE) ? 0.0f : dt.count());
		d->Draw(wnd.Gfx());
	}
	// imgui stuff
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static bool show_demo_window = true;
	if (show_demo_window)
	{
		ImGui::ShowDemoWindow(&show_demo_window);
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	wnd.Gfx().EndFrame();
	angle += 0.001f;
}
