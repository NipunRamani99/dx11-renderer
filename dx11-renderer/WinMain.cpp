#include "MinWin.hpp"
#include "WindowMessageMap.hpp"
#include "Window.hpp"
#include <sstream>
#include "App.hpp"
#include <filesystem>
#include <iostream>
int CALLBACK WinMain(
	HINSTANCE,
	HINSTANCE,
	LPSTR lpCmdLine,
	int)
{
	std::string current = std::filesystem::current_path().string();
	char buffer[MAX_PATH];

	if (GetCurrentDirectoryA(MAX_PATH, buffer) == 0) {
		std::cerr << "Error getting current working directory: " << GetLastError() << std::endl;
		return 1;
	}

	std::string currentDir = buffer;

	std::cout << "Current Working Directory: " << currentDir << std::endl;

	try {
		return App{ std::string{lpCmdLine} }.Go();
	} catch (Exception & e) {
		MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
	} catch (std::exception& e) {
		MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
	} 
	catch (...) {
		MessageBox(nullptr, "No Exception Detail", "Unknown Exception Thrown", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}