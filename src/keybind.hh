#ifndef __KRANEWM__KEYBIND__GUARD__
#define __KRANEWM__KEYBIND__GUARD__

#include "commands.hh"

#include "x-data/key.hh"

#include <unordered_map>


struct keyshortcut_t
{
    keyshortcut_t(KeySym _keysym, unsigned _mask)
        : keysym(_keysym),
          mask(_mask) {}

    keyshortcut_t(XKeyEvent event)
        : keysym(x_data::get_keysym(event.keycode)),
          mask(event.state) {}

    inline bool operator==(const keyshortcut_t& ks) const
    {
        return ks.keysym == keysym && ks.mask == mask;
    }

    KeySym keysym;
    unsigned mask;
};

namespace std
{
    template <>
    struct hash<keyshortcut_t>
    {
        std::size_t operator()(const keyshortcut_t& ks) const
        {
            return ks.keysym + 10000 * ks.mask;
        }
    };
}

typedef ::std::unordered_map<keyshortcut_t, commandbind_t> keybinds_t;

#endif//__KRANEWM__KEYBIND__GUARD__
