#pragma once
#include "MinWin.hpp"

class GDIPlusManager
{
  private:
    static ULONG_PTR token;
    static int refCount;

  public:
    GDIPlusManager();
    ~GDIPlusManager();
};
