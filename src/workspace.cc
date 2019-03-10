#include "workspace.hh"


void
user_workspace_t::arrange() const
{

}

user_workspace_t&
user_workspace_t::add_client(client_ptr_t client)
{
    clients.add(client);
    ::std::for_each(client->children.begin(), client->children.end(),
        [=](client_ptr_t child) { clients.add(child); });

    return *this;
}

user_workspace_t&
user_workspace_t::remove_client(client_ptr_t client)
{
    ::std::for_each(client->children.begin(), client->children.end(),
        [=](client_ptr_t child) { clients.remove(child); });
    clients.remove(client);

    return *this;
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
