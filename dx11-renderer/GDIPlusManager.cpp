#define FULL_WIN
#include "MinWin.hpp"
#include <algorithm>
#include "GDIPlusManager.hpp"

namespace Gdiplus
{
using std::max;
using std::min;
} // namespace Gdiplus
#include <gdiplus.h>
ULONG_PTR GDIPlusManager::token = 0;
int GDIPlusManager::refCount    = 0;

GDIPlusManager::GDIPlusManager()
{
    if( refCount++ == 0 )
    {
        Gdiplus::GdiplusStartupInput input;
        input.GdiplusVersion           = 1;
        input.DebugEventCallback       = nullptr;
        input.SuppressBackgroundThread = false;
        Gdiplus::GdiplusStartup( &token, &input, nullptr );
    }
}

GDIPlusManager::~GDIPlusManager()
{
    --refCount;
    if( refCount == 0 )
    {
        Gdiplus::GdiplusShutdown( token );
    }
}
