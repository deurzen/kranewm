#ifndef __KRANEWM__MOUSEBIND__GUARD__
#define __KRANEWM__MOUSEBIND__GUARD__

#include "x-wrapper/mouse.hh"

#include <unordered_map>


enum class mouseop_t
{
    noop,
    client_move,
    client_resize,
    client_center,
    client_next_ws,
    client_prev_ws,
    goto_next_ws,
    goto_prev_ws,
};

struct mouseshortcut_t
{
    mouseshortcut_t(unsigned _button , unsigned _mask, bool _on_client)
        : button(_button),
          mask(_mask),
          on_client(_on_client) {}

    inline bool operator==(const mouseshortcut_t& ms) const
    {
        return ms.button == button && ms.mask == mask && ms.on_client == on_client;
    }

    unsigned button;
    unsigned mask;
    bool on_client;
};

namespace std
{
    template <>
    struct hash<mouseshortcut_t>
    {
        std::size_t operator()(const mouseshortcut_t& ms) const
        {
            return ms.button + 10000 * ms.mask + ms.on_client;
        }
    };
}

typedef ::std::unordered_map<mouseshortcut_t, mouseop_t> mousebinds_t;

#endif//__KRANEWM__MOUSEBIND__GUARD__
