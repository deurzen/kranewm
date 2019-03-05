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
    public:
        static ::std::map<::std::string, Atom> g_interned_atoms;

        atom_t() = default;

        atom_t(Atom atom)
            : val(atom)
        {}

        atom_t(const ::std::string& name)
            : val(XInternAtom(g_dpy, name.c_str(), False))
        {}

        atom_t(const char* name)
            : val(XInternAtom(g_dpy, name, False))
        {}

        operator Atom() const { return val; }
        operator bool() const { return val != 0; }

        inline int  length() const { return 1; }
        inline Atom type()   const { return XA_ATOM; }
        inline int  size()   const { return 32; }

    private:
        Atom val;

    };

    extern atom_t get_atom(::std::string name);
}

#endif//__KRANEWM__X_WRAPPER__ATOM__GUARD__
