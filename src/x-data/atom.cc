#include "atom.hh"

using namespace x_data;


::std::map<::std::string, Atom> atom_t::g_interned_atoms;
::std::map<Atom, ::std::string> atom_t::g_atom_names;

atom_t
x_data::get_atom(const ::std::string& name)
{
    if (atom_t::g_interned_atoms.count(name) > 0)
        return atom_t::g_interned_atoms[name];

    Atom atom = XInternAtom(g_dpy, name.c_str(), False);
    x_data::intern_atom(name, atom);
    return atom;
}

void
x_data::intern_atom(const ::std::string& name, atom_t atom)
{
    atom_t::g_interned_atoms[name] = atom;
    atom_t::g_atom_names[atom] = name;
}

const ::std::string
x_data::get_atom_name(atom_t atom)
{
    if (atom_t::g_atom_names.count(atom) > 0)
        return atom_t::g_atom_names[atom];

    return {};
}
