#pragma once
#include <unordered_map>
#include <string>
#include <Windows.h>
class WindowsMessageMap
{
  public:
    WindowsMessageMap ();
    std::string operator() ( DWORD msg, LPARAM lparam, WPARAM wparam ) const;

  private:
    std::unordered_map<DWORD, std::string> msg_map;
};
