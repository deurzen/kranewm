#include "workspace.hh"


void
user_workspace_t::arrange() const
{

}

user_workspace_t&
user_workspace_t::add_client(client_ptr_t)
{

    return *this;
}

user_workspace_t&
user_workspace_t::remove_client(client_ptr_t)
{

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
user_workspace_t::set_layout(LayoutType _layout)
{
    LayoutType current_layout = layout;
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

}

void
user_workspace_t::unmap_clients()
{

}

void
user_workspace_t::activate()
{

}

void
user_workspace_t::deactivate()
{

}
