#pragma once
// Error Helper Macro
#define HWND_EXCEPT( hr ) Window::HrException ( __LINE__, __FILE__, hr )
#define HWND_LAST_EXCEPT() Window::HrException ( __LINE__, __FILE__, GetLastError () )
#define HWND_NO_GFX_EXCEPT() Window::NoGfxException ( __LINE__, __FILE__ )
