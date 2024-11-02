#include "Window.hpp"
#include <sstream>
#include "resource.h"
#include "WindowsThrowMacros.h"
#include "imgui/imgui_impl_win32.h"
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
	DWORD style = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX ;
	//Calculate the window size based on the desired client area size;
	RECT wr{ 0 };
	wr.left = 0;
	wr.right = wr.left + width;
	wr.top = 0;
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
	ImGui_ImplWin32_Init(hwnd);
	// create graphics object
	pGfx = std::make_unique<Graphics>(hwnd);

	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid.usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	Rid.dwFlags = 0;
	Rid.hwndTarget = 0;


	if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) == FALSE)
	{
		throw HWND_LAST_EXCEPT();
	}
}

Window::~Window()
{
	ImGui_ImplWin32_Shutdown();
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

POINT Window::GetCenterPosition()
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	rect.left += SCREEN_WIDTH >> 1;
	rect.top += SCREEN_HEIGHT >> 1;
	POINT centerPoint{ rect.left, rect.top };
	ScreenToClient(hwnd, &centerPoint);
	return centerPoint;
}

void Window::CenterCursorPosition()
{
	RECT rect;
	GetWindowRect(hwnd, &rect);
	rect.left += SCREEN_WIDTH >> 1;
	rect.top += SCREEN_HEIGHT >> 1;
	SetCursorPos(rect.left, rect.top);
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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
	const auto& imio = ImGui::GetIO();

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
			if (imio.WantCaptureKeyboard)
			{
				break;
			}
			if (!(lparam & 0x40000000) || kbd.AutorepeatIsEnabled()) {
				kbd.OnKeyPressed(static_cast<unsigned char>(wparam));
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (imio.WantCaptureKeyboard)
			{
				break;
			}
			kbd.OnKeyReleased(static_cast<unsigned char>(wparam));
			break;
		case WM_CHAR:
			if (imio.WantCaptureKeyboard)
			{
				break;
			}
			kbd.OnChar(static_cast<unsigned char>(wparam));
			break;
	/*************** END KEYBOARD INPUT MESSAGES ***************/

	/*************** MOUSE INPUT MESSAGES ***************/
		case WM_MOUSEMOVE:
		{
			if (imio.WantCaptureMouse)
			{
				break;
			}
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
			if (imio.WantCaptureMouse)
			{
				break;
			}
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnLeftPressed(pt.x, pt.y);
			break;
		}
		case WM_LBUTTONUP: 
		{
			if (imio.WantCaptureMouse)
			{
				break;
			}
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
			if (imio.WantCaptureMouse)
			{
				break;
			}
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnRightPressed(pt.x, pt.y);
			break;
		}
		case WM_RBUTTONUP:
		{
			if (imio.WantCaptureMouse)
			{
				break;
			}
			const POINTS pt = MAKEPOINTS(lparam);
			mouse.OnRightReleased(pt.x, pt.y);
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height)
			{
				ReleaseCapture();
				mouse.OnMouseLeave();
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			if (imio.WantCaptureMouse)
			{
				break;
			}
			const POINTS pt = MAKEPOINTS(lparam);
			const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
			mouse.OnWheelDelta(pt.x, pt.y, delta);
			break;
		}
		case WM_INPUT:
		{
			unsigned int size = 0;
			if (GetRawInputData((HRAWINPUT)lparam,
				RID_INPUT,
				nullptr,
				&size,
				sizeof(RAWINPUTHEADER)) == -1)
			{
				// bail if read failed
				OutputDebugString(TEXT("GetRawInputData failed for some reason !\n"));
				break;
			}
			rawInputBuffer.resize(size);
			char* data = rawInputBuffer.data();
			if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT,
				data, &size,
				sizeof(RAWINPUTHEADER)) != size)
			{
				OutputDebugString(TEXT("GetRawInputData does not return correct size !\n"));
				break;
			}
			RAWINPUT* raw = (RAWINPUT*)data;
			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				int x = raw->data.mouse.lLastX;
				int y = raw->data.mouse.lLastY;
				mouse.OnMouseRawInput(x, y);
			}
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
