#include "atom.hh"

using namespace x_wrapper;


::std::map<::std::string, Atom> atom_t::g_interned_atoms;

atom_t
x_wrapper::get_atom(::std::string name)
{
    if (atom_t::g_interned_atoms.count(name) > 0)
        return atom_t::g_interned_atoms[name];
    Atom atom = XInternAtom(g_dpy, name.c_str(), False);
    atom_t::g_interned_atoms[name] = atom;
    return atom;
}
