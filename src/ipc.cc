#include "ipc.hh"

bool
ipc_t::assert_target(x_data::event_t event) const
{
    return !x_data::get_atom_name(event.get().xproperty.atom).rfind(IPC_PREFIX, 0);
}

ipccommand_t
ipc_t::resolve_operation(x_data::event_t event) const
{
    auto name = x_data::get_atom_name(event.get().xproperty.atom);
    name = name.substr(IPC_PREFIX.size(), name.size() - IPC_PREFIX.size());

    if (supported_commands.count(name) > 0) {
        const auto [atom,op] = supported_commands.at(name);
        return {{}, op};
    }

    return {{}, ipcop_t::noop};
}
