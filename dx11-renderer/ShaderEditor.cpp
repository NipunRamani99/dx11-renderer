#include "imgui\imgui.h"
#include "ShaderEditor.hpp"
#include "imfilebrowser.h"
#include "ImGuiColorTextEdit\TextEditor.h"
#include <exception>
#include <iostream>

ShaderEditor::ShaderEditor()
{
	_fileBrowser = std::make_unique<ImGui::FileBrowser>();
	_fileBrowser->SetTypeFilters({ ".hlsl"});
	_textEditor = std::make_unique<TextEditor>();
	auto lang = TextEditor::LanguageDefinition::HLSL();
	_textEditor->SetLanguageDefinition(lang);
}

ShaderEditor::~ShaderEditor()
{
}

void ShaderEditor::Show(const char* title)
{
	if (!ImGui::Begin(title, nullptr, ImGuiWindowFlags_MenuBar))
	{
		_file.close();
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
			if (ImGui::MenuItem("Save"))
			{
				SaveFile();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	if(_fileOpen)
	{
		_textEditor->Render("Shader Editor");
	}
	ImGui::End();
	
	_fileBrowser->Display();
	if (_fileBrowser->HasSelected())
	{
		_filePath = _fileBrowser->GetSelected().string();
		_fileBrowser->ClearSelected();
		try
		{
			LoadFile(_filePath);
			_fileOpen = true;
		}
		catch (std::fstream::failure e)
		{
			std::string error = e.what();
			std::cout << e.what();
		}
	}
}  

void ShaderEditor::LoadFile(const std::string& path)
{
	if (_file)
		_file.close();
	_file.open(path, std::fstream::in | std::fstream::out);
	std::stringstream sstream;
	sstream << _file.rdbuf();
	_text = sstream.str();
	_textEditor->SetText(_text);
}

void ShaderEditor::SaveFile()
{
	if (_file.is_open())
	{
		_file.seekg(0);
		_file << _textEditor->GetText();
	}
}