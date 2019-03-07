#ifndef __KRANEWM__MOUSEBIND__GUARD__
#define __KRANEWM__MOUSEBIND__GUARD__

#include "x_wrapper/mouse.hh"

#include <map>


enum MouseOperation
{
    MOUSE_NOOP = 0,
    CLIENT_MOVE,
    CLIENT_RESIZE,
    CLIENT_CENTER,
    CLIENT_NEXT_WS,
    CLIENT_PREV_WS,
    GOTO_NEXT_WS,
    GOTO_PREV_WS,
};

struct MouseShortcut
{
    MouseShortcut(unsigned _button , unsigned _mask, bool _on_client)
        : button(_button),
          mask(_mask),
          on_client(_on_client)
    {}

    unsigned button;
    unsigned mask;
    bool on_client;
};

inline bool
operator<(const MouseShortcut& ms1, const MouseShortcut& ms2)
{
    auto cmp1 = ms1.button + 10000 * ms1.mask + ms1.on_client;
    auto cmp2 = ms2.button + 10000 * ms2.mask + ms2.on_client;
    return cmp1 < cmp2;
}

typedef ::std::map<MouseShortcut, MouseOperation> MouseBinds;

#endif//__KRANEWM__MOUSEBIND__GUARD__
