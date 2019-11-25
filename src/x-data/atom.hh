#ifndef __KRANEWM__X_DATA__ATOM__GUARD__
#define __KRANEWM__X_DATA__ATOM__GUARD__

#include "common.hh"
#include "type.hh"
#include "display.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>
}

#include <cstring>
#include <map>
#include <string>
#include <vector>


namespace x_data
{
    class atom_t : public x_type
    {
    public:
        static ::std::map<::std::string, Atom> g_interned_atoms;
        static ::std::map<Atom, ::std::string> g_atom_names;

        atom_t() = default;

        atom_t(Atom atom)
            : val(atom) {}

        atom_t(const ::std::string& name)
            : val(XInternAtom(g_dpy, name.c_str(), False)) {}

        atom_t(const char* name)
            : val(XInternAtom(g_dpy, name, False)) {}

        explicit atom_t(void* raw_data, unsigned long _)
            : val(*(Atom*)raw_data) {}

        operator Atom() const { return val; }
        operator bool() const { return val != 0; }

        inline bool operator==(const atom_t& atom) const
        {
            return atom.val == val;
        }

        inline bool operator==(const Atom& atom) const
        {
            return atom == val;
        }

        inline int  length() const { return 1; }
        inline Atom type()   const { return XA_ATOM; }
        inline int  size()   const { return 32; }

        inline Atom get() const { return val; }
        inline const Atom* get_ptr() const { return &val; }

    private:
        Atom val;

    };

    class atom_list_t : public x_type
    {
    public:
        atom_list_t() = default;

        atom_list_t(Atom* atom_list, ::std::size_t list_len)
            : len(list_len)
        {
            for (::std::size_t i = 0; i < list_len; ++i)
                val.push_back(atom_list[i]);
        }

        atom_list_t(void* raw_data, unsigned long data_len)
            : len(data_len)
        {
            Atom* atom_list = (Atom*) raw_data;
            for (::std::size_t i = 0; i < data_len; ++i)
                val.push_back(atom_list[i]);
        }

        operator ::std::vector<Atom>() const { return val; }
        operator Atom*() { return val.data(); }
        operator bool() const { return !val.empty(); }

        inline bool operator==(const atom_list_t& atom_list) const
        {
            if (atom_list.len != len)
                return false;

            bool different = false;
            for (::std::size_t i = 0; i < len; ++i)
                if (val[i] != atom_list.val[i])
                    different = true;

            return !different;
        }

        inline int  length() const { return len; }
        inline Atom type()   const { return XA_ATOM; }
        inline int  size()   const { return 32; }

        inline Atom* get() { return &val[0]; }
        inline Atom* get_ptr() { return val.data(); }

    private:
        ::std::vector<Atom> val;
        ::std::size_t len;

    };

    extern atom_t get_atom(const ::std::string& name);
    extern void intern_atom(const ::std::string& name, atom_t atom);
    extern const ::std::string get_atom_name(atom_t atom);
}

#endif//__KRANEWM__X_DATA__ATOM__GUARD__
