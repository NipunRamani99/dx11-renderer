#include "MinWin.hpp"
#include "WindowMessageMap.hpp"
#include "Window.hpp"
#include <sstream>
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static WindowsMessageMap wmm;
	OutputDebugString(wmm(msg, lParam, wParam).c_str());
	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == 'F')
			SetWindowText(hwnd, "KEYDOWN");
		break;
	case WM_KEYUP:
		if (wParam == 'F')
			SetWindowText(hwnd, "KEYUP");
		break;
	case WM_CHAR:
		{
		static std::string title;
		title.push_back((char)wParam);
		SetWindowText(hwnd, title.c_str());
		}
		break;
	case WM_LBUTTONDOWN:
		{
		const POINTS pt = MAKEPOINTS(lParam);
		std::ostringstream oss;
		oss << "(" << pt.x << ", " << pt.y << ")" << "\n";
		SetWindowText(hwnd, oss.str().c_str());
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//Class Name
	const auto pClassName = "DX11_Renderer";
	const auto pWindowName = "My DX11 Renderer";
	Window wnd(640, 480, pWindowName);
	MSG msg;
	BOOL gResult;
	while ((gResult = GetMessage(&msg, nullptr, 0, 0))> 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (gResult == -1) {
		return -1;
	}
	else {
		return msg.wParam;
	}
}