#include "Mouse.hpp"
#include <Windows.h>

std::pair<int, int> Mouse::GetPos() const noexcept
{
    return { x, y };
}

int Mouse::GetPosX() const noexcept
{
    return x;
}

int Mouse::GetPosY() const noexcept
{
    return y;
}

bool Mouse::IsInWindow() const noexcept
{
    return isInWindow;
}

bool Mouse::LeftIsPressed() const noexcept
{
    return leftIsPressed;
}

bool Mouse::RightIsPressed() const noexcept
{
    return rightIsPressed;
}

Mouse::Event Mouse::Read() noexcept
{
    if( buffer.size() > 0u )
    {
        Mouse::Event e = buffer.front();
        buffer.pop();
        return e;
    }
    else
    {
        return Mouse::Event();
    }
}

std::optional<Mouse::RawInputEvent> Mouse::ReadRaw() noexcept
{
    if( rawBuffer.size() > 0u )
    {
        RawInputEvent rawevent = rawBuffer.front();
        rawBuffer.pop();
        return std::optional( rawevent );
    }
    else
    {
        return std::nullopt;
    }
}

void Mouse::Flush() noexcept
{
    buffer    = std::queue<Event>();
    rawBuffer = std::queue<RawInputEvent>();
}

void Mouse::OnMouseMove( int newx, int newy ) noexcept
{
    x = newx;
    y = newy;

    buffer.push( Mouse::Event( Mouse::Event::Type::Move, *this ) );
    TrimBuffer();
}

void Mouse::OnMouseLeave() noexcept
{
    isInWindow = false;
    buffer.push( Mouse::Event( Mouse::Event::Type::Leave, *this ) );
    TrimBuffer();
}

void Mouse::OnMouseEnter() noexcept
{
    isInWindow = true;
    buffer.push( Mouse::Event( Mouse::Event::Type::Enter, *this ) );
    TrimBuffer();
}

void Mouse::OnLeftPressed( int, int ) noexcept
{
    leftIsPressed = true;

    buffer.push( Mouse::Event( Mouse::Event::Type::LPress, *this ) );
    TrimBuffer();
}

void Mouse::OnLeftReleased( int, int ) noexcept
{
    leftIsPressed = false;

    buffer.push( Mouse::Event( Mouse::Event::Type::LRelease, *this ) );
    TrimBuffer();
}

void Mouse::OnRightPressed( int, int ) noexcept
{
    rightIsPressed = true;

    buffer.push( Mouse::Event( Mouse::Event::Type::RPress, *this ) );
    TrimBuffer();
}

void Mouse::OnRightReleased( int, int ) noexcept
{
    rightIsPressed = false;

    buffer.push( Mouse::Event( Mouse::Event::Type::RRelease, *this ) );
    TrimBuffer();
}

void Mouse::OnWheelUp( int, int ) noexcept
{
    buffer.push( Mouse::Event( Mouse::Event::Type::WheelUp, *this ) );
    TrimBuffer();
}

void Mouse::OnWheelDown( int, int ) noexcept
{
    buffer.push( Mouse::Event( Mouse::Event::Type::WheelDown, *this ) );
    TrimBuffer();
}

void Mouse::OnMouseRawInput( int dx, int dy ) noexcept
{
    RawInputEvent rawEvent( dx, dy );
    rawBuffer.push( rawEvent );
}

void Mouse::TrimBuffer() noexcept
{
    while( buffer.size() > bufferSize )
    {
        buffer.pop();
    }
}

void Mouse::TrimRawBuffer() noexcept
{
    while( rawBuffer.size() > bufferSize )
    {
        rawBuffer.pop();
    }
}

void Mouse::OnWheelDelta( int px, int py, int delta ) noexcept
{
    wheelDeltaCarry += delta;
    // generate events for every 120
    while( wheelDeltaCarry >= WHEEL_DELTA )
    {
        wheelDeltaCarry -= WHEEL_DELTA;
        OnWheelUp( px, py );
    }
    while( wheelDeltaCarry <= -WHEEL_DELTA )
    {
        wheelDeltaCarry += WHEEL_DELTA;
        OnWheelDown( px, py );
    }
}
