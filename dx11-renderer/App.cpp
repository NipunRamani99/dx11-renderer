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
#include "Vertex.h"
#include "AABBVisualisation.hpp"
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
	std::generate_n(std::back_inserter(drawables), nDrawables, factory);

}

App::App()
	:
	imgui(),
	wnd(SCREEN_WIDTH, SCREEN_HEIGHT, "The Donkey Fart Box"),
	light(wnd.Gfx())
{
	GeometryAssortmentScene(wnd.Gfx(), drawables, nDrawables);
	wnd.Gfx().SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
	wnd.Gfx().SetCamera(DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	model = std::make_unique<Model>(wnd.Gfx(), "models/nanosuit.gltf");
}

int App::Go()
{
	wnd.CenterCursorPosition();


	while (keepRunning) {

		if (!_showCursor)
		{
			wnd.CenterCursorPosition();
		}
		if (auto ecode = wnd.ProcessMessage()) {
			return *ecode;
		}

		float currentMouseX = float(wnd.mouse.GetPosX());
		float currentMouseY = float(wnd.mouse.GetPosY());

		float deltaX = currentMouseX - prevMouseX;
		float deltaY =  prevMouseY - currentMouseY;
		
		if(!_showCursor)
			_fpsCam.Update(wnd.kbd, -deltaX, deltaY);
		
		DoFrame();


		if (wnd.kbd.KeyIsPressed(VK_ESCAPE)) {
			keepRunning = false;
		}
		
		if (wnd.kbd.KeyIsPressed('K'))
		{
			_showCursor = !_showCursor;
			//ShowCursor(_showCursor ? TRUE : FALSE);
			if(_showCursor)
				ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
			else
				ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
		}
	}
	return 0;
}


void App::DoFrame()
{
	static AABB aabb{};

	static AABBVisualisation aabbviz(wnd.Gfx(), aabb);
	static float angle = 0.0f;
	const float c = 0.5f;
	wnd.Gfx().SetCamera(_fpsCam.GetMatrix());
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
	model->Draw(wnd.Gfx());
	model->DrawAABB(wnd.Gfx());
	light.Draw(wnd.Gfx());

	if (wnd.Gfx().IsImguiEnabled()) {

		// imgui window to control simulation speed
		if (ImGui::Begin("Simulation Speed"))
		{
			ImGui::SliderFloat("Speed Factor", &speed_factor, 0.0f, 4.0f);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Status: %s", wnd.kbd.KeyIsPressed(VK_SPACE) ? "PAUSED" : "RUNNING");
		}

		ImGui::Text("Yaw: %.2f", _fpsCam._yaw);
		ImGui::Text("Pitch: %.2f", _fpsCam._pitch);
		ImGui::Text("Mouse : %d %d", wnd.mouse.GetPosX(), wnd.mouse.GetPosY());
		ImGui::Text("Prev Mouse X: %.2f", prevMouseX);
		ImGui::Text("Prev Mouse Y: %.2f", prevMouseY);
		ImGui::Text("Mouse clicked: %d", wnd.mouse.LeftIsPressed());

		ImGui::End();
		static ImGuiTextBuffer Buf;
		if (ImGui::Begin("Event Logs"))
		{
			
			const Mouse::Event event = wnd.mouse.Read();
			std::string eventStr = event.ToString();
			Buf.append(eventStr.cbegin()._Ptr, eventStr.cend()._Ptr);
			Buf.append("\n");
			ImGui::TextUnformatted(Buf.begin());
			ImGui::SetScrollHereY(1.0f);
			
		}
		ImGui::End();
	}
	cam.SpawnControl();
	light.SpawnControlWindow();
	wnd.Gfx().EndFrame();
	//angle += 0.001f;
}
