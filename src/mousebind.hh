#ifndef __KRANEWM__MOUSEBIND__GUARD__
#define __KRANEWM__MOUSEBIND__GUARD__

#include "x-data/mouse.hh"

#include <unordered_map>
#include <utility>


enum class mouseop_t
{
    noop,
    focus_fwd,
    focus_bck,
    client_move,
    client_resize,
    center_client,
    client_next_ws,
    client_prev_ws,
    goto_next_ws,
    goto_prev_ws,
    goto_next_cx,
    goto_prev_cx,
    toggle_float,
    toggle_fullscreen,
    toggle_disown,
};

enum class mousetarget_t
{
    root = 1,
    sidebar,
    client,
};

struct mouseshortcut_t
{
    mouseshortcut_t(unsigned _button , unsigned _mask, mousetarget_t _target)
        : button(_button),
          mask(_mask),
          target(_target) {}

    inline bool operator==(const mouseshortcut_t& ms) const
    {
        return ms.button == button && ms.mask == mask && ms.target == target;
    }

    unsigned button;
    unsigned mask;
    mousetarget_t target;
};

namespace std
{
    template <>
    struct hash<mouseshortcut_t>
    {
        std::size_t operator()(const mouseshortcut_t& ms) const
        {
            return ms.button + 10000 * ms.mask + static_cast<int>(ms.target);
        }
    };
}

typedef ::std::pair<mouseop_t, bool> mouseaction_t;
typedef ::std::unordered_map<mouseshortcut_t, mouseaction_t> mousebinds_t;

#endif//__KRANEWM__MOUSEBIND__GUARD__
