#include "App.hpp"
#include <algorithm>
#include "ChiliMath.hpp"
#include "GDIPlusManager.hpp"
#include "Surface.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "Vertex.h"
#include "AABBVisualisation.hpp"
#include "TaskManager.hpp"
GDIPlusManager gdipm;

App::App()
	:
	imgui(),
	wnd(SCREEN_WIDTH, SCREEN_HEIGHT, "DX11 Renderer"),
	light(wnd.Gfx())
{
	projection = DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f);
	wnd.Gfx().SetProjection(projection);
	wnd.Gfx().SetCamera(DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f));
	model = std::make_unique<Model>(wnd.Gfx(), "./models/nanosuit/nanosuit.obj", 2.0f);
	//pokeWall = std::make_unique<Model>(wnd.Gfx(), "./models/flat_wall/flatwall.gltf");

	//plane = std::make_unique<TestPlane>(wnd.Gfx());
	//model->Transform(DirectX::XMMatrixRotationRollPitchYaw(0.0f, DirectX::XMConvertToRadians(180.0f), 0.0f));
	//pokeWall->Transform(DirectX::XMMatrixTranslation(2.0f, 10.0f, -3.0f));
	//pokeWall->Transform(DirectX::XMMatrixRotationRollPitchYaw(DirectX::XMConvertToRadians(-90.0f), 0.0f, 0.0f));
	gobber = std::make_unique<Model>(wnd.Gfx(), "./models/Gobber/GoblinX.obj");
	gobber->Transform(DirectX::XMMatrixTranslation(0.0f, 7.0f, -7.0f));
}

int App::Go()
{
	POINT center = wnd.GetCenterPosition();
	prevMouseX = center.x;
	prevMouseY = center.y;

	while (keepRunning) {
		if (auto ecode = wnd.ProcessMessage()) 
		{
			return *ecode;
		}
		if (!_showCursor)
		{
			wnd.CenterCursorPosition();

			// Handle input for strafing (left/right)
			float dt = 1.0f / 60.0f;
			if (wnd.kbd.KeyIsPressed('A'))
			{
				_fpsCam.Translate({ 1.0f, 0.0f, 0.0f });
			}
			if (wnd.kbd.KeyIsPressed('D'))
			{
				_fpsCam.Translate({ -1.0f, 0.0f, 0.0f });
			}

			if (wnd.kbd.KeyIsPressed('W'))
			{
				_fpsCam.Translate({ 0.0f, 0.0f, 1.0f});
			}
			if (wnd.kbd.KeyIsPressed('S'))
			{
				_fpsCam.Translate({ 0.0f, 0.0f, -1.0f});
			}
		}
		float currentMouseX = float(wnd.mouse.GetPosX());
		float currentMouseY = float(wnd.mouse.GetPosY());
		while (auto event = wnd.mouse.ReadRaw())
		{
			float deltaX = event->GetDeltaX();
			float deltaY = event->GetDeltaY();

			if (!_showCursor)
				_fpsCam.Update(-deltaX, -deltaY);
		}
		if (_showCursor && wnd.mouse.LeftIsPressed())
		{
			roFloat = { currentMouseX, currentMouseY, 0.0f };
			rdFloat = { currentMouseX, currentMouseY, 1.0f };
			DirectX::XMVECTOR roMouse = DirectX::XMLoadFloat3(&roFloat);
			DirectX::XMVECTOR rdMouse = DirectX::XMLoadFloat3(&rdFloat);
			DirectX::XMVECTOR ro = DirectX::XMVector3Unproject(roMouse, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, projection, _fpsCam.GetMatrix(), DirectX::XMMatrixIdentity());
			DirectX::XMVECTOR rd = DirectX::XMVector3Unproject(rdMouse, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 1.0f, projection, _fpsCam.GetMatrix(),
				DirectX::XMMatrixIdentity());
			rd = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(rd, ro));

			DirectX::XMStoreFloat3(&roFloat, ro);
			DirectX::XMStoreFloat3(&rdFloat, rd);
		}

		DoFrame();

		if (wnd.kbd.KeyIsPressed(VK_ESCAPE)) 
		{
			keepRunning = false;
		}
		TaskManager & manager = TaskManager::Get();
		if (wnd.kbd.KeyIsPressed('K'))
		{
			if (canToggle)
			{
				_showCursor = !_showCursor;
				ShowCursor(_showCursor ? TRUE : FALSE);
				if (_showCursor)
					ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
				else
					ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
				canToggle = false;
			}
		}
		else
		{
			canToggle = true;
		}
	}
	return 0;
}


void App::DoFrame()
{
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
	float currentMouseX = float(wnd.mouse.GetPosX());
	float currentMouseY = float(wnd.mouse.GetPosY());
	DirectX::XMFLOAT3 pos = _fpsCam.GetPos();
	DirectX::XMFLOAT3 dir = _fpsCam.GetDir();

	wnd.Gfx().BeginFrame(c, c, 1.0f);
	light.Bind(wnd.Gfx(), _fpsCam.GetMatrix());
	//plane->Draw(wnd.Gfx());
	//plane->SpawnControl();
	model->Draw(wnd.Gfx());
	//model->DrawAABB(wnd.Gfx());
	model->ShowWindow();
	//pokeWall->Draw(wnd.Gfx());
	//pokeWall->DrawAABB(wnd.Gfx());
	//pokeWall->ShowWindow();
	/*gobber->Draw(wnd.Gfx());
	gobber->DrawAABB(wnd.Gfx());
	gobber->ShowWindow();*/
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
		ImGui::Text("Ray Hit: %d", result.hit);
		ImGui::Text("Node selected: %s", result.node ? result.node->GetName().c_str() : "None");
		ImGui::Text("Cam Pos: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
		ImGui::Text("Cam Dir: %.2f, %.2f, %.2f", dir.x, dir.y, dir.z);

		ImGui::Text("RO: %.2f, %.2f, %.2f", roFloat.x, roFloat.y, roFloat.z);
		ImGui::Text("RD: %.2f, %.2f, %.2f", rdFloat.x, rdFloat.y, rdFloat.z);

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
