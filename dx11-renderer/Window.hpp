#pragma once
#include "MinWin.hpp"

class Window {
private:
	int width = 0;
	int height = 0;
	HWND hwnd;
	//Singleton class to register/cleanup the win32 window class.
	class WindowClass {
	public:
		static const char* GetName() noexcept;
		static HINSTANCE GetInstance() noexcept;
	private:
		WindowClass() noexcept;
		~WindowClass();
		WindowClass(const Window& window) = delete;
		WindowClass& operator=(const WindowClass&) = delete;
		static constexpr const char* wndClassName = "My Dx11 Renderer";
		static WindowClass wndClass;
		HINSTANCE hInst;
	};
public:
	Window(int width, int height, const char * name) noexcept;
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window& ) = delete;
private:
	static LRESULT WINAPI HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	static LRESULT WINAPI HandleMsgProxy(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
};