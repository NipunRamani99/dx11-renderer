#include "Window.hpp"
#include <sstream>
#include "resource.h"
#include "WindowsThrowMacros.h"
Window::WindowClass Window::WindowClass::wndClass;
Window::WindowClass::WindowClass() noexcept
	:
	hInst(GetModuleHandle(nullptr))
{

	HANDLE hIcon = LoadImage(hInst, MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 16, 16, 0);
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetInstance();
	wc.hIcon = static_cast<HICON>(hIcon);
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
	:
	width(width),
	height(height)
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
	// create graphics object
	pGfx = std::make_unique<Graphics>(hwnd);
}

Window::~Window()
{
	DestroyWindow(hwnd);
}

std::optional<int> Window::ProcessMessage() noexcept
{
	MSG msg;
	// while queue has messages, remove and dispatch them (but do not block on empty queue)
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		// check for quit because peekmessage does not signal this via return val
		if (msg.message == WM_QUIT)
		{
			// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
			return (int)msg.wParam;
		}

		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// return an empty optional when not quitting app
	return {};
}

void Window::SetTitle(const std::string& title)
{
	if (SetWindowText(hwnd, title.c_str()) == 0)
	{
		throw HWND_LAST_EXCEPT();
	}
}

Graphics& Window::Gfx()
{
	if (!pGfx) {
		throw HWND_NO_GFX_EXCEPT();
	}
	return *pGfx;
}

void Window::CenterCursorPosition()
{
	//SetCursorPos(width,height);
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
		case WM_KILLFOCUS:
			kbd.ClearState();
			break;
	/*************** KEYBOARD INPUT MESSAGES ***************/
		case WM_KEYDOWN:
		// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		case WM_SYSKEYDOWN:
			if (!(lparam & 0x40000000) || kbd.AutorepeatIsEnabled()) {
				kbd.OnKeyPressed(static_cast<unsigned char>(wparam));
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			kbd.OnKeyReleased(static_cast<unsigned char>(wparam));
			break;
		case WM_CHAR:
			kbd.OnChar(static_cast<unsigned char>(wparam));
			break;
	/*************** END KEYBOARD INPUT MESSAGES ***************/

	/*************** MOUSE INPUT MESSAGES ***************/
		case WM_MOUSEMOVE:
		{
			const POINTS pt = MAKEPOINTS(lparam);
			if (pt.x >= 0 && pt.y >= 0 && pt.x < width && pt.y < width) {
				mouse.OnMouseMove(pt.x, pt.y);
				if (!mouse.IsInWindow())
				{
					SetCapture(hwnd);
					mouse.OnMouseEnter();
				}
			}
			else {
				if (wparam & (MK_LBUTTON | MK_RBUTTON)) {
					mouse.OnMouseMove(pt.x, pt.y);
				}
				else {
					ReleaseCapture();
					mouse.OnMouseLeave();
				}
			}
			break;
		}
		case WM_LBUTTONDOWN: 
		{
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnLeftPressed(pt.x, pt.y);
			break;
		}
		case WM_LBUTTONUP: 
		{
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnLeftReleased(pt.x, pt.y);
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height)
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
			break;
		}
		case WM_RBUTTONDOWN:
		{	
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnRightPressed(pt.x, pt.y);
			break;
		}
		case WM_RBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnRightReleased(pt.x, pt.y);
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height)
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
			break;
		}case WM_MOUSEWHEEL:
		{
			const POINTS pt = MAKEPOINTS(lparam);
			const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
			mouse.OnWheelDelta(pt.x, pt.y, delta);
			break;
		}

	/*************** END MOUSE INPUT MESSAGES ***************/
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

Window::HrException::HrException(int line, const char* file, HRESULT hr) noexcept
	:
	Exception(line, file),
	hr(hr)
{
}

const char* Window::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] " << GetErrorCode() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Window::HrException::GetType() const noexcept
{
	return "Window Exception";
}

std::string Window::HrException::TranslateErrorCode(HRESULT hr)
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

HRESULT Window::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Window::HrException::GetErrorDescription() const noexcept
{
	return TranslateErrorCode(hr);
}

const char* Window::NoGfxException::GetType() const noexcept
{
	return "No Graphics Exception";
}
