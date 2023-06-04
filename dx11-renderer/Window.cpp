#include "Window.hpp"
#include <sstream>
Window::WindowClass Window::WindowClass::wndClass;
Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr))
{
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = GetName();
	wc.hIconSm = nullptr;
	RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass()
{
	UnregisterClass(wndClassName, GetInstance());
}

const char* Window::WindowClass::GetName() noexcept
{
	return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept
{
	return wndClass.hInst;
}
Window::Window(int width, int height, const char* name)
{
	//Required Style
	DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	//Calculate the window size based on the desired client area size;
	RECT wr{ 0 };
	wr.left = 100;
	wr.right = wr.left + width;
	wr.top = 100;
	wr.bottom = wr.top + height;
	if (FAILED(AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE)))
	{
		throw HWND_LAST_EXCEPT();
	};
	// create window & get hwnd
	hwnd = CreateWindow(WindowClass::GetName(), name, style, CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, WindowClass::GetInstance(), this);
	if (hwnd == nullptr) {
		throw HWND_LAST_EXCEPT();
	}
	ShowWindow(hwnd, SW_SHOWDEFAULT);

}

Window::~Window()
{
	DestroyWindow(hwnd);
}

LRESULT __stdcall Window::HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	if (msg == WM_NCCREATE) {
		const CREATESTRUCT* const pCreate = reinterpret_cast<CREATESTRUCT*>(lparam);
		Window* const pwnd = static_cast<Window*>(pCreate->lpCreateParams);
		// set WinAPI-managed User data to store ptr to window class
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pwnd));
		// set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgProxy));
		// forward message to window class handler
		return pwnd->HandleMsg(hwnd, msg, wparam, lparam);
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT __stdcall Window::HandleMsgProxy(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	//Retrieve ptr to Window Class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	return pWnd->HandleMsg(hwnd, msg, wparam, lparam);
}

LRESULT Window::HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	switch (msg) {
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

Window::WindowException::WindowException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{
}

const char* Window::WindowException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] " << GetErrorCode() << std::endl
		<< "[Description] " << GetErrorString() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::WindowException::GetType() const noexcept
{
	return "Window Exception";
}

std::string Window::WindowException::TranslateErrorCode(HRESULT hr)
{
	char* errorMsg = nullptr;
	DWORD msgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&errorMsg), 0, nullptr
	);
	if (msgLen == 0)
	{
		return "Unidentified error code";
	}
	std::string strErrorMsg = errorMsg;
	return strErrorMsg;
}

HRESULT Window::WindowException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::WindowException::GetErrorString() const noexcept
{
	return TranslateErrorCode(hr);
}


