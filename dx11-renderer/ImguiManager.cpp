#include "ImguiManager.hpp"
#include "imgui/imgui.h"

ImguiManager::ImguiManager()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

ImguiManager::~ImguiManager()
{
    ImGui::DestroyContext();
}
