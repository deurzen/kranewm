#ifndef __KRANEWM__X_WRAPPER__ATOM__GUARD__
#define __KRANEWM__X_WRAPPER__ATOM__GUARD__

#include "type.hh"
#include "display.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>
}

#include <string>
#include <map>


namespace x_wrapper
{
    class atom_t : protected x_type
    {
        friend atom_t get_atom(::std::string);

    public:
        atom_t(Atom atom)
            : val(atom)
        {}

        operator Atom() const { return val; }
        operator bool() const { return val != 0; }

        inline int size() const { return 1; }
        inline Atom type() const { return XA_ATOM; }

    private:
        Atom val;

        static ::std::map<::std::string, Atom> g_interned_atoms;

    };

    atom_t get_atom(::std::string name) {
        if (atom_t::g_interned_atoms.count(name) > 0)
            return atom_t::g_interned_atoms[name];
        Atom atom = XInternAtom(g_dpy, name.c_str(), False);
        atom_t::g_interned_atoms[name] = atom;
        return atom;
    }
}

#endif//__KRANEWM__X_WRAPPER__ATOM__GUARD__
