#pragma once
#include <string>
#include <fstream>
#include <memory>
namespace ImGui
{
	class FileBrowser;
}
class TextEditor;
class ShaderEditor
{
private:
	std::string _text = "";
	std::string _filePath = "";
	std::fstream _file;
	std::unique_ptr<ImGui::FileBrowser> _fileBrowser;
	std::unique_ptr<TextEditor> _textEditor;
	bool _fileOpen = false;
public:
	ShaderEditor();
	void Show(const char* title);
	~ShaderEditor();
private:
	void LoadFile(const std::string& path);
	void SaveFile();
};