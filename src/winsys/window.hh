#ifndef __WINSYS_WINDOW_H_GUARD__
#define __WINSYS_WINDOW_H_GUARD__

#include <cstdlib>

namespace winsys
{

    typedef std::size_t Window;

    enum class IcccmWindowState
    {
        Withdrawn,
        Normal,
        Iconic
    };

    enum class WindowState
    {
        Modal,
        Sticky,
        MaximizedVert,
        MaximizedHorz,
        Shaded,
        SkipTaskbar,
        SkipPager,
        Hidden,
        Fullscreen,
        Above_,
        Below_,
        DemandsAttention
    };

    enum class WindowType
    {
        Desktop,
        Dock,
        Toolbar,
        Menu,
        Utility,
        Splash,
        Dialog,
        DropdownMenu,
        PopupMenu,
        Tooltip,
        Notification,
        Combo,
        Dnd,
        Normal
    };

}

#endif//__WINSYS_WINDOW_H_GUARD__
