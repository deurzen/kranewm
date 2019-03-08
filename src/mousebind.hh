#ifndef __KRANEWM__MOUSEBIND__GUARD__
#define __KRANEWM__MOUSEBIND__GUARD__

#include "x-wrapper/mouse.hh"

#include <unordered_map>


enum mouseop
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

struct mouseshortcut
{
    mouseshortcut(unsigned _button , unsigned _mask, bool _on_client)
        : button(_button),
          mask(_mask),
          on_client(_on_client)
    {}

    inline bool operator==(const mouseshortcut& ms) const
    {
        return ms.button == button && ms.mask == mask;
    }

    unsigned button;
    unsigned mask;
    bool on_client;
};

namespace std
{
    template <>
    struct hash<mouseshortcut>
    {
        std::size_t operator()(const mouseshortcut& ms) const
        {
            return ms.button + 10000 * ms.mask;
        }
    };
}

typedef ::std::unordered_map<mouseshortcut, mouseop> mousebinds;

#endif//__KRANEWM__MOUSEBIND__GUARD__
