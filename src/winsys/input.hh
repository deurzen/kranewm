#ifndef __WINSYS_INPUT_H_GUARD__
#define __WINSYS_INPUT_H_GUARD__

#include "common.hh"
#include "window.hh"
#include "geometry.hh"

#include <unordered_set>
#include <optional>
#include <numeric>

namespace winsys
{

    enum class Key
    {
        Any,
		BackSpace,
		Tab,
		Clear,
		Return,
		Shift,
		Control,
		Alt,
		Super,
		Menu,
		Pause,
		CapsLock,
		Escape,
		Space,
        ExclamationMark,
        QuotationMark,
        QuestionMark,
        NumberSign,
        DollarSign,
        PercentSign,
        AtSign,
        Ampersand,
        Apostrophe,
        LeftParenthesis,
        RightParenthesis,
        LeftBracket,
        RightBracket,
        LeftBrace,
        RightBrace,
        Underscore,
        Grave,
        Bar,
        Tilde,
        QuoteLeft,
        Asterisk,
        Plus,
        Comma,
        Minus,
        Period,
        Slash,
        BackSlash,
        Colon,
        SemiColon,
        Less,
        Equal,
        Greater,
		PageUp,
		PageDown,
		End,
		Home,
		Left,
		Up,
		Right,
		Down,
		Select,
		Print,
		Execute,
		PrintScreen,
		Insert,
		Delete,
		Help,
		Zero,
		One,
		Two,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		NumPad0,
		NumPad1,
		NumPad2,
		NumPad3,
		NumPad4,
		NumPad5,
		NumPad6,
		NumPad7,
		NumPad8,
		NumPad9,
		Multiply,
		Add,
		Seperator,
		Subtract,
		Decimal,
		Divide,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		Numlock,
		ScrollLock,
		LeftShift,
		RightShift,
		LeftControl,
		RightContol,
		LeftAlt,
		RightAlt,
		LeftSuper,
		RightSuper,
		BrowserBack,
		BrowserForward,
		BrowserRefresh,
		BrowserStop,
		BrowserSearch,
		BrowserFavorites,
		BrowserHome,
		VolumeMute,
		VolumeDown,
		VolumeUp,
		MicMute,
		NextTrack,
		PreviousTrack,
		StopMedia,
		PlayPause,
		BrightnessDown,
		BrightnessUp,
		KeyboardBrightnessDown,
		KeyboardBrightnessUp,
		LaunchMail,
		SelectMedia,
		LaunchAppA,
		LaunchAppB,
		LaunchAppC,
		LaunchAppD,
		LaunchAppE,
		LaunchAppF,
		LaunchApp0,
		LaunchApp1,
		LaunchApp2,
		LaunchApp3,
		LaunchApp4,
		LaunchApp5,
		LaunchApp6,
		LaunchApp7,
		LaunchApp8,
		LaunchApp9
    };

    enum class Button
    {
        Left,
        Middle,
        Right,
        ScrollUp,
        ScrollDown,
        Backward,
        Forward
    };

    enum Modifier
    {
        Ctrl       = 1 << 0,
        Shift      = 1 << 1,
        Alt        = 1 << 2,
        AltGr      = 1 << 3,
        Super      = 1 << 4,
        NumLock    = 1 << 5,
        ScrollLock = 1 << 6,
#ifndef DEBUG
        Main       = Super,
        Sec        = Alt
#else
        Main       = Alt,
        Sec        = Super
#endif
    };

    inline Modifier
    operator|(Modifier lhs, Modifier rhs)
    {
        return static_cast<Modifier>(
            static_cast<std::size_t>(lhs)
                | static_cast<std::size_t>(rhs)
        );
    }

    struct KeyInput final
    {
        Key key;
        std::unordered_set<Modifier> modifiers;
    };

    struct KeyCapture final
    {
        KeyInput input;
        std::optional<Window> window;
    };

    struct MouseInput final
    {
        enum class MouseInputTarget
        {
            Global,
            Root,
            Client
        };

        MouseInputTarget target;
        Button button;
        std::unordered_set<Modifier> modifiers;
    };

    struct MouseCapture final
    {
        enum class MouseCaptureKind
        {
            Press,
            Release,
            Motion
        };

        MouseCaptureKind kind;
        MouseInput input;
        std::optional<Window> window;
        Pos root_rpos;
    };

    enum Grip
    {
        Left   = 1 << 0,
        Right  = 1 << 1,
        Top    = 1 << 2,
        Bottom = 1 << 3,
    };

    inline Grip
    operator|(Grip lhs, Grip rhs)
    {
        return static_cast<Grip>(
            static_cast<std::size_t>(lhs)
                | static_cast<std::size_t>(rhs)
        );
    }

    inline Grip&
    operator|=(Grip& lhs, Grip rhs)
    {
        lhs = static_cast<Grip>(
            static_cast<std::size_t>(lhs)
                | static_cast<std::size_t>(rhs)
        );

        return lhs;
    }

    inline Grip
    operator&(Grip lhs, Grip rhs)
    {
        return static_cast<Grip>(
            static_cast<std::size_t>(lhs)
                & static_cast<std::size_t>(rhs)
        );
    }

    inline Grip&
    operator&=(Grip& lhs, Grip rhs)
    {
        lhs = static_cast<Grip>(
            static_cast<std::size_t>(lhs)
                & static_cast<std::size_t>(rhs)
        );

        return lhs;
    }

    inline bool
    operator==(KeyInput const& lhs, KeyInput const& rhs)
    {
        return lhs.key == rhs.key
            && lhs.modifiers == rhs.modifiers;
    }

    inline bool
    operator==(MouseInput const& lhs, MouseInput const& rhs)
    {
        return lhs.target == rhs.target
            && lhs.button == rhs.button
            && lhs.modifiers == rhs.modifiers;
    }

}

namespace std
{
    template <>
    struct hash<winsys::KeyInput>
    {
        std::size_t
        operator()(winsys::KeyInput const& input) const
        {
            std::size_t key_hash = std::hash<winsys::Key>()(input.key);
            std::size_t modifiers_hash = std::hash<std::size_t>()(
                std::accumulate(
                    input.modifiers.begin(),
                    input.modifiers.end(),
                    static_cast<winsys::Modifier>(0),
                    std::bit_or<winsys::Modifier>()
                )
            );

            return key_hash ^ modifiers_hash;
        }
    };
}

namespace std
{
    template <>
    struct hash<winsys::MouseInput>
    {
        std::size_t
        operator()(winsys::MouseInput const& input) const
        {
            std::size_t target_hash = std::hash<winsys::MouseInput::MouseInputTarget>()(input.target);
            std::size_t button_hash = std::hash<winsys::Button>()(input.button);
            std::size_t modifiers_hash = std::hash<std::size_t>()(
                std::accumulate(
                    input.modifiers.begin(),
                    input.modifiers.end(),
                    static_cast<winsys::Modifier>(0),
                    std::bit_or<winsys::Modifier>()
                )
            );

            return target_hash ^ button_hash ^ modifiers_hash;
        }
    };
}

#endif//__WINSYS_INPUT_H_GUARD__
