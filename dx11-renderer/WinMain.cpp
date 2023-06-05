#include "MinWin.hpp"
#include "WindowMessageMap.hpp"
#include "Window.hpp"
#include <sstream>
#include "App.hpp"
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	
	try {
		return App{}.Go();
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