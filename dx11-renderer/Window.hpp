#pragma once
#include "MinWin.hpp"
#include "Exception.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include <optional>
class Window {
private:
	int width = 0;
	int height = 0;
	HWND hwnd;
public:
	Keyboard kbd;
	Mouse mouse;
private:
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
	class WindowException : public Exception{
	public:
		WindowException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		static std::string TranslateErrorCode(HRESULT hr);
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
	private:
		HRESULT hr;
	};

public:
	Window(int width, int height, const char * name);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window& ) = delete;
	std::optional<int> ProcessMessage();
	void SetTitle(const std::string& str);
private:
	static LRESULT WINAPI HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	static LRESULT WINAPI HandleMsgProxy(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
};

//Error Helper Macro
#define HWND_EXCEPT( hr ) Window::WindowException( __LINE__, __FILE__, hr)
#define HWND_LAST_EXCEPT() Window::WindowException( __LINE__,__FILE__,GetLastError() )