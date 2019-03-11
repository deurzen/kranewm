#include "workspace.hh"


void
user_workspace_t::arrange() const
{
    switch (layout) {
    case layout_t::floating:   layouthandler.layout_floating(*this);   break;
    case layout_t::tile:       layouthandler.layout_tile(*this);       break;
    case layout_t::deck:       layouthandler.layout_deck(*this);       break;
    case layout_t::doubledeck: layouthandler.layout_doubledeck(*this); break;
    case layout_t::grid:       layouthandler.layout_grid(*this);       break;
    case layout_t::monocle:    layouthandler.layout_monocle(*this);    break;
    default: break;
    }
}

unsigned
user_workspace_t::get_number() const
{
    return number;
}

const ::std::deque<client_ptr_t>&
user_workspace_t::get_all() const
{
    return clients.get_all();
}

bool
user_workspace_t::empty() const
{
    return clients.get_all().empty();
}

bool
user_workspace_t::contains(client_ptr_t client) const
{
    return clients.contains(client);
}

bool
user_workspace_t::in_float_layout() const
{
    return layout == layout_t::floating;
}

bool
user_workspace_t::is_mirrored() const
{
    return mirrored;
}

const
client_ptr_t
user_workspace_t::get_focused() const
{
    return clients.get();
}

void
user_workspace_t::set_focused(client_ptr_t client)
{
    clients.set(client);
}

void
user_workspace_t::unset_focused()
{
    clients.unset();
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
user_workspace_t::forward()
{
    clients.next_focus();
    return *this;
}

user_workspace_t&
user_workspace_t::backward()
{
    clients.prev_focus();
    return *this;
}

user_workspace_t&
user_workspace_t::zoom()
{
    if (!empty())
    {
        auto zoomed = clients.zoom();
        if (in_float_layout() && zoomed.first && zoomed.second) {
            pos_t pos1 = zoomed.first->float_pos, pos2 = zoomed.second->float_pos;
            dim_t dim1 = zoomed.first->float_dim, dim2 = zoomed.second->float_dim;
            zoomed.first->resize(dim2).move(pos2);
            zoomed.second->resize(dim1).move(pos1);
        }
    }

    return *this;
}

user_workspace_t&
user_workspace_t::mirror()
{
    mirrored = !mirrored;
}

user_workspace_t&
user_workspace_t::set_n_master(unsigned new_n_master)
{
    n_master = new_n_master;
    return *this;
}

user_workspace_t&
user_workspace_t::set_gap_size(unsigned new_gap_size)
{
    gap_size = new_gap_size;
    return *this;
}

user_workspace_t&
user_workspace_t::set_m_factor(float new_m_factor)
{
    m_factor = new_m_factor;
    return *this;
}

user_workspace_t&
user_workspace_t::set_m1_weight(unsigned new_m1_weight)
{
    m1_weight = new_m1_weight;
    return *this;
}

user_workspace_t&
user_workspace_t::set_layout(layout_t _layout)
{
    layout_t current_layout = layout;
    if (_layout == layout_t::toggle)
        layout = previous_layout;
    else
        layout = _layout;

    previous_layout = current_layout;
    return *this;
}


unsigned
user_workspace_t::get_n_master() const
{
    return n_master;
}

unsigned
user_workspace_t::get_gap_size() const
{
    return gap_size;
}

float
user_workspace_t::get_m_factor() const
{
    return m_factor;
}

unsigned
user_workspace_t::get_m1_weight() const
{
    return m1_weight;
}

layout_t
user_workspace_t::get_layout() const
{
    return layout;
}
