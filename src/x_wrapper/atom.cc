#include "atom.hh"

::std::map<::std::string, Atom> x_wrapper::atom_t::g_interned_atoms;

x_wrapper::atom_t x_wrapper::get_atom(::std::string name) {
    if (x_wrapper::atom_t::g_interned_atoms.count(name) > 0)
        return x_wrapper::atom_t::g_interned_atoms[name];
    Atom atom = XInternAtom(x_wrapper::g_dpy, name.c_str(), False);
    x_wrapper::atom_t::g_interned_atoms[name] = atom;
    return atom;
}
