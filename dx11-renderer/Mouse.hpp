#pragma once
#include <queue>
#include <string>
#include <optional>
class Mouse
{
    friend class Window;

  public:
    class Event
    {
      public:
        enum Type
        {
            LPress,
            LRelease,
            RPress,
            RRelease,
            WheelUp,
            WheelDown,
            Move,
            Enter,
            Leave,
            Invalid
        };

      private:
        Type type;
        bool leftIsPressed;
        bool rightIsPressed;
        int x;
        int y;

      public:
        Event () noexcept : type ( Type::Invalid ), leftIsPressed ( false ), rightIsPressed ( false ), x ( 0 ), y ( 0 )
        {
        }
        Event ( Type type, const Mouse& parent ) noexcept
            : type ( type ), leftIsPressed ( parent.leftIsPressed ), rightIsPressed ( parent.rightIsPressed ),
              x ( parent.x ), y ( parent.y )
        {
        }
        bool IsValid () const noexcept
        {
            return type != Type::Invalid;
        }
        Type GetType () const noexcept
        {
            return type;
        }
        std::pair<int, int> GetPos () const noexcept
        {
            return { x, y };
        }
        int GetPosX () const noexcept
        {
            return x;
        }
        int GetPosY () const noexcept
        {
            return y;
        }
        bool LeftIsPressed () const noexcept
        {
            return leftIsPressed;
        }
        bool RightIsPressed () const noexcept
        {
            return rightIsPressed;
        }

        const std::string ToString () const
        {
            std::string eventStr = "";
            switch ( type )
            {
            case Move:
                eventStr = "MOUSE_MOVE";
                break;
            case LPress:
                eventStr = "LMB_PRESS";
                break;
            case RPress:
                eventStr = "RMB_PRESS";
                break;
            case LRelease:
                eventStr = "LMB_RELEASE";
                break;
            case RRelease:
                eventStr = "RMB_RELEASE";
                break;
            case Enter:
                eventStr = "MOUSE_ENTER";
                break;
            case Leave:
                eventStr = "MOUSE_LEAVE";
                break;
            case WheelUp:
                eventStr = "MOUSE_WHEEL_UP";
                break;
            case WheelDown:
                eventStr = "MOUSE_WHEEL_DOWN";
                break;
            case Invalid:
                eventStr = "MOUSE_INVALID";
                break;
            default:
                eventStr = "MOUSE_UNKNOWN_TYPE";
                break;
            }

            eventStr += " X: " + std::to_string ( x ) + " Y: " + std::to_string ( y );
            return eventStr;
        }
    };

    class RawInputEvent
    {
      private:
        int x = 0;
        int y = 0;

      public:
        RawInputEvent ( int x, int y ) : x ( x ), y ( y ) {}

        long GetDeltaX () noexcept
        {
            return x;
        }

        long GetDeltaY () noexcept
        {
            return y;
        }

        const std::string ToString ()
        {
            return "Mouse Delta X: " + std::to_string ( x ) + " Y:" + std::to_string ( y );
        }
    };

  private:
    static constexpr unsigned int bufferSize = 16u;
    int x;
    int y;
    bool leftIsPressed  = false;
    bool rightIsPressed = false;
    bool isInWindow     = false;
    int wheelDeltaCarry = 0;
    std::queue<Event> buffer;
    std::queue<RawInputEvent> rawBuffer;

  public:
    Mouse ()                          = default;
    Mouse ( const Mouse& )            = delete;
    Mouse& operator= ( const Mouse& ) = delete;
    std::pair<int, int> GetPos () const noexcept;
    int GetPosX () const noexcept;
    int GetPosY () const noexcept;
    bool IsInWindow () const noexcept;
    bool LeftIsPressed () const noexcept;
    bool RightIsPressed () const noexcept;
    Mouse::Event Read () noexcept;
    std::optional<RawInputEvent> ReadRaw () noexcept;
    bool IsEmpty () const noexcept
    {
        return buffer.empty ();
    }
    void Flush () noexcept;

    const std::queue<Event>& GetEventBuffer ()
    {
        return buffer;
    }

  private:
    void OnMouseMove ( int x, int y ) noexcept;
    void OnMouseLeave () noexcept;
    void OnMouseEnter () noexcept;
    void OnLeftPressed ( int, int ) noexcept;
    void OnLeftReleased ( int, int ) noexcept;
    void OnRightPressed ( int, int ) noexcept;
    void OnRightReleased ( int, int ) noexcept;
    void OnWheelUp ( int, int ) noexcept;
    void OnWheelDown ( int, int ) noexcept;
    void OnMouseRawInput ( int dx, int dy ) noexcept;
    void TrimBuffer () noexcept;
    void TrimRawBuffer () noexcept;
    void OnWheelDelta ( int x, int y, int delta ) noexcept;
};
