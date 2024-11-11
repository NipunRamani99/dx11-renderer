#include "imgui\imgui.h"
#include "Graphics.hpp"
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
			if (ImGui::BeginMenu("Compile"))
			{
				if(ImGui::MenuItem("Pixel Shader"))
					CompileShader("ps");
				if (ImGui::MenuItem("Vertex Shader"))
					CompileShader("vs");
				ImGui::EndMenu();
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

void ShaderEditor::CompileShader(std::string shaderType)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	// /E"main" /Fo"C:\projects\dx11\dx11-renderer\dx11-renderer\ColorBlendPS.cso" /ps"_5_0" /nologo
	size_t offset =  _filePath.find_last_of(".");
	std::string output(_filePath.begin(), _filePath.begin() + offset);
	output = output + ".cso";
	std::string fxcPath = "C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.22621.0\\x86\\fxc.exe";
	std::string commandLine = "/E\"main\" /Fo\""+output+" /"+shaderType+"\"_5_0\" /nologo " + _filePath;
	if (!FAILED(CreateProcessA(fxcPath.c_str(), (LPSTR)commandLine.c_str(),
		NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)))
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
	}
	else
	{
		DWORD errorMessageID = GetLastError();
		if (errorMessageID == 0) {
			std::cout << "No error has occurred." << std::endl;
			return;
		}

		// Buffer for the error message
		LPSTR messageBuffer = nullptr;

		// Format the message from the system
		size_t size = FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorMessageID,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&messageBuffer,
			0,
			NULL
		);

		int x = 100;
	}
}
