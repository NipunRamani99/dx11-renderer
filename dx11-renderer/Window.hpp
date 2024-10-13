#pragma once
#include "MinWin.hpp"
#include "Exception.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include <optional>
#include <memory>
#include "Graphics.hpp"
#include "WindowsConstants.h"

class Window {
private:
	unsigned int width = SCREEN_WIDTH;
	unsigned int height = SCREEN_HEIGHT;
	HWND hwnd;
	std::unique_ptr<Graphics> pGfx;
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
	class HrException : public Exception{
	public:
		HrException(int line, const char* file, HRESULT hr) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		static std::string TranslateErrorCode(HRESULT hr);
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorDescription() const noexcept;
	private:
		HRESULT hr;
	};
	class NoGfxException : public Exception {
	public:
		using Exception::Exception;
		const char* GetType() const noexcept override;
	};
public:
	Window(int width, int height, const char * name);
	~Window();
	Window(const Window&) = delete;
	Window& operator=(const Window& ) = delete;
	std::optional<int> ProcessMessage() noexcept;
	void SetTitle(const std::string& str);
	Graphics& Gfx();
	void CenterCursorPosition();
	POINT GetCenterPosition();

private:
	static LRESULT WINAPI HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	static LRESULT WINAPI HandleMsgProxy(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
};

