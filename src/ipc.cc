#include "ipc.hh"

bool
ipc_t::assert_target(x_data::event_t event) const
{
    return !x_data::get_atom_name(event.get().xproperty.atom).rfind(IPC_PREFIX, 0);
}
