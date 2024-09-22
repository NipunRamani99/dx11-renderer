#include "App.hpp"
#include "Box.hpp"
#include "SolidSphere.hpp"
#include "Pyramid.hpp"
#include "Cone.hpp"
#include <algorithm>
#include "ChiliMath.hpp"
#include "GDIPlusManager.hpp"
#include "Surface.hpp"
#include "SkinnedBox.hpp"
#include "AssimpTest.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "VertexLayout.h"
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
		std::uniform_real_distribution<float> normDist{ 0.0f, 1.0f };

	public:
		DrawableFactory(Graphics& gfx)
			:
			gfx(gfx)
		{
		}

		std::unique_ptr<Drawable> operator()() {
			DirectX::XMFLOAT3 material{normDist(rng),normDist(rng),normDist(rng)};
			return std::make_unique<AssimpTest>(
				gfx, rng, adist, ddist,
				odist, rdist, material, 1.5f
			);
		}

	};
	DrawableFactory factory(gfx);
	//box = std::make_unique<Box>(wnd.Gfx());
	drawables.reserve(nDrawables);
	std::generate_n(std::back_inserter(drawables), 1, factory);

}

App::App()
	:
	imgui(),
	wnd(800, 600, "The Donkey Fart Box"),
	light(wnd.Gfx())
{
	GeometryAssortmentScene(wnd.Gfx(), drawables, nDrawables);
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
	wnd.Gfx().SetCamera(DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	using namespace hw3dexp;
	VertexLayout layout;
	layout.Append<VertexLayout::Position3D>();

	layout.Append<VertexLayout::Normal>();

	auto& element = layout.Resolve<VertexLayout::Normal>();
	assert(element.GetOffset() == sizeof(DirectX::XMFLOAT3));

	assert(layout.Size() == sizeof(DirectX::XMFLOAT3) + sizeof(DirectX::XMFLOAT3));

	VertexBuffer buffer(layout);

	buffer.EmplaceBack( DirectX::XMFLOAT3{ 1.0f, 2.0f, 3.0f }, DirectX::XMFLOAT3{ 1.0f, 2.0f, 3.0f } );

	assert(buffer.Size() == 1);

	ConstVertex vert = buffer.Back();
	const DirectX::XMFLOAT3 & pos = vert.Attr<VertexLayout::Position3D>();
	assert(pos.x == 1.0f && pos.y == 2.0f && pos.z == 3.0f);
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
	wnd.Gfx().SetCamera(cam.GetMatrix());
	if (wnd.kbd.KeyIsPressed(VK_SPACE))
	{
		wnd.Gfx().DisableImgui();
	}
	else
	{
		wnd.Gfx().EnableImgui();
	}
	wnd.Gfx().BeginFrame(c, c, 1.0f);
	light.Bind(wnd.Gfx(), cam.GetMatrix());
	float mouseX = 2.0f* wnd.mouse.GetPosX()/800.0f - 1;
	float mouseY = -2.0f*wnd.mouse.GetPosY()/600.0f + 1;
	auto dt = timer.Mark().count() * speed_factor;
	/*wnd.Gfx().DrawTestTriangle(
		timer.Peek().count(),
		wnd.mouse.GetPosX() / 400.0f - 1.0f,
		-wnd.mouse.GetPosY() / 300.0f + 1.0f
	);*/
	//box->Update(dt.count());
	//box->Draw(wnd.Gfx());
	
	for (auto& d : drawables)
	{
		d->Update(wnd.kbd.KeyIsPressed(VK_SPACE) ? 0.0f : dt);
		d->Draw(wnd.Gfx());
	}
	light.Draw(wnd.Gfx());
	if (wnd.Gfx().IsImguiEnabled()) {
		static char buffer[1024];

		// imgui window to control simulation speed
		if (ImGui::Begin("Simulation Speed"))
		{
			ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 4.0f);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Status: %s", wnd.kbd.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING");
		}
		ImGui::End();
	}
	cam.SpawnControl();
	light.SpawnControlWindow();
	wnd.Gfx().EndFrame();
	angle += 0.001f;
}
