#include "imgui\imgui.h"
#include "ShaderEditor.hpp"
#include "imfilebrowser.h"

ShaderEditor::ShaderEditor()
{
	_fileBrowser = std::make_unique<ImGui::FileBrowser>();
	_fileBrowser->SetTypeFilters({ ".hlsl"});
}

ShaderEditor::~ShaderEditor()
{
}

void ShaderEditor::Show(const char* title)
{
	if (!ImGui::Begin(title, nullptr, ImGuiWindowFlags_MenuBar))
	{
		ImGui::End();
		return;
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				_fileBrowser->Open();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::End();
	
	_fileBrowser->Display();

	if (_fileBrowser->HasSelected())
	{
		_filePath = _fileBrowser->GetSelected().string();
		_fileBrowser->ClearSelected();
	}
}  

void ShaderEditor::LoadFile(const std::string& path)
{
	_file.open(path, std::fstream::in | std::fstream::out);
	_text.clear();
	_file >> _text;
}

void ShaderEditor::SaveFile()
{
	if (_file.is_open())
	{
		_file.seekg(0);
		_file << _text;
	}
}