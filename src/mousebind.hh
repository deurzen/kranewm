#ifndef __KRANEWM__MOUSEBIND__GUARD__
#define __KRANEWM__MOUSEBIND__GUARD__

#include "commands.hh"

#include "x-data/mask.hh"
#include "x-data/mouse.hh"

#include <unordered_map>
#include <utility>
#include <variant>


enum class mousetarget_t
{
    client = 1,
    root,
    sidebar,
};

struct mouseshortcut_t
{
    mouseshortcut_t(unsigned _button , unsigned _mask, mousetarget_t _target)
        : button(_button),
          mask(_mask),
          target(_target) {}

    inline bool operator==(const mouseshortcut_t& ms) const
    {
        return ms.button == button && x_data::clean_mask(ms.mask) == x_data::clean_mask(mask) && ms.target == target;
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
        ::std::size_t operator()(const mouseshortcut_t& ms) const
        {
            return ms.button + 10000 * x_data::clean_mask(ms.mask) + static_cast<int>(ms.target);
        }
    };
}

typedef ::std::pair<commandbind_t, bool> mouseaction_t;
typedef ::std::unordered_map<mouseshortcut_t, mouseaction_t> mousebinds_t;

#endif//__KRANEWM__MOUSEBIND__GUARD__
