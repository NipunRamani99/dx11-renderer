#include "MinWin.hpp"
#include "WindowMessageMap.hpp"
#include "Window.hpp"
#include <sstream>
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//Class Name
	const auto pClassName = "DX11_Renderer";
	const auto pWindowName = "My DX11 Renderer";
	
	try {
		Window wnd(640, 480, pWindowName);
		MSG msg;
		BOOL gResult;
		while ((gResult = GetMessage(&msg, nullptr, 0, 0)) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (gResult == -1) {
			throw HWND_LAST_EXCEPT();
		}
		else {
			return msg.wParam;
		}
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