#include "Exception.hpp"
#include <sstream>
Exception::Exception( int line, const char* file ) noexcept : line( line ), file( file ) {}

const char* Exception::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Exception::GetType() const noexcept
{
    return "BaseException";
}

int Exception::GetLine() const noexcept
{
    return line;
}

const std::string& Exception::GetFile() const noexcept
{
    return file;
}

std::string Exception::GetOriginString() const noexcept
{
    std::ostringstream oss;
    oss << "[FILE] " << file << std::endl << "[LINE] " << line;
    return oss.str();
}
