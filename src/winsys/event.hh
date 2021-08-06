#ifndef __WINSYS_EVENT_H_GUARD__
#define __WINSYS_EVENT_H_GUARD__

#include "common.hh"
#include "geometry.hh"
#include "input.hh"
#include "window.hh"

#include <cstdlib>
#include <optional>
#include <variant>

namespace winsys
{

    enum class StackMode
    {
        Above_,
        Below_
    };

    enum class PropertyKind
    {
        Name,
        Class,
        Size,
        Strut
    };

    struct MouseEvent final
    {
        MouseCapture capture;
        bool on_root;
    };

    struct KeyEvent final
    {
        KeyCapture capture;
    };

    struct MapRequestEvent final
    {
        Window window;
        bool ignore;
    };

    struct MapEvent final
    {
        Window window;
        bool ignore;
    };

    struct EnterEvent final
    {
        Window window;
        Pos root_rpos;
        Pos window_rpos;
    };

    struct LeaveEvent final
    {
        Window window;
        Pos root_rpos;
        Pos window_rpos;
    };

    struct DestroyEvent final
    {
        Window window;
    };

    struct ExposeEvent final
    {
        Window window;
    };

    struct UnmapEvent final
    {
        Window window;
        bool ignore;
    };

    struct StateRequestEvent final
    {
        Window window;
        WindowState state;
        Toggle action;
        bool on_root;
    };

    struct FocusRequestEvent final
    {
        Window window;
        bool on_root;
    };

    struct CloseRequestEvent final
    {
        Window window;
        bool on_root;
    };

    struct WorkspaceRequestEvent final
    {
        std::optional<Window> window;
        std::size_t index;
        bool on_root;
    };

    struct PlacementRequestEvent final
    {
        Window window;
        std::optional<Pos> pos;
        std::optional<Dim> dim;
        bool on_root;
    };

    struct GripRequestEvent final
    {
        Window window;
        Pos pos;
        std::optional<Grip> grip;
        bool on_root;
    };

    struct RestackRequestEvent final
    {
        Window window;
        Window sibling;
        StackMode mode;
        bool on_root;
    };

    struct ConfigureEvent final
    {
        Window window;
        Region region;
        bool on_root;
    };

    struct PropertyEvent final
    {
        Window window;
        PropertyKind kind;
        bool on_root;
    };

    struct FrameExtentsRequestEvent final
    {
        Window window;
        bool on_root;
    };

    struct ScreenChangeEvent final {};

    typedef std::variant<
        std::monostate,
        MouseEvent,
        KeyEvent,
        MapRequestEvent,
        MapEvent,
        EnterEvent,
        LeaveEvent,
        DestroyEvent,
        ExposeEvent,
        UnmapEvent,
        StateRequestEvent,
        FocusRequestEvent,
        CloseRequestEvent,
        WorkspaceRequestEvent,
        PlacementRequestEvent,
        GripRequestEvent,
        RestackRequestEvent,
        ConfigureEvent,
        PropertyEvent,
        FrameExtentsRequestEvent,
        ScreenChangeEvent
    > Event;

}

#endif//__WINSYS_EVENT_H_GUARD__
