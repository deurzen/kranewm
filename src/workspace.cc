#include "workspace.hh"


void
user_workspace_t::arrange() const
{

}

user_workspace_t&
user_workspace_t::set_n_master(unsigned)
{

    return *this;
}

user_workspace_t&
user_workspace_t::set_gap_size(unsigned)
{

    return *this;
}

user_workspace_t&
user_workspace_t::set_m_factor(float)
{

    return *this;
}

user_workspace_t&
user_workspace_t::set_m1_weight(unsigned)
{

    return *this;
}

user_workspace_t&
user_workspace_t::set_layout(layouttype _layout)
{
    layouttype current_layout = layout;
    if (_layout == LT_TOGGLE)
        layout = previous_layout;
    else
        layout = _layout;

    previous_layout = current_layout;
    return *this;
}

void
user_workspace_t::map_clients()
{
    for (auto& client : clients.get_all()) {
        client->expect = MAP;
        client->map_children().map();
    }
}

void
user_workspace_t::unmap_clients()
{
    for (auto& client : clients.get_all()) {
        client->expect = WITHDRAW;
        client->unmap_children().unmap();
    }
}

void
user_workspace_t::activate()
{

}

void
user_workspace_t::deactivate()
{

}
