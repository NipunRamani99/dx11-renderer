#pragma once
#include <exception>
#include <string>

class Exception : public std::exception
{
  private:
    std::string file;
    int line;

  protected:
    mutable std::string whatBuffer;

  public:
    Exception ( int line, const char* file ) noexcept;
    virtual const char* what () const noexcept;
    virtual const char* GetType () const noexcept;
    int GetLine () const noexcept;
    const std::string& GetFile () const noexcept;
    std::string GetOriginString () const noexcept;
};
