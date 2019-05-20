#include "workspace.hh"


void
user_workspace_t::arrange() const
{
    switch (m_layout) {
    case layout_t::floating:   m_layouthandler.layout_floating(*this);   break;
    case layout_t::tile:       m_layouthandler.layout_tile(*this);       break;
    case layout_t::deck:       m_layouthandler.layout_deck(*this);       break;
    case layout_t::doubledeck: m_layouthandler.layout_doubledeck(*this); break;
    case layout_t::grid:       m_layouthandler.layout_grid(*this);       break;
    case layout_t::monocle:    m_layouthandler.layout_monocle(*this);    break;
    case layout_t::pillar:     m_layouthandler.layout_pillar(*this);     break;
    default: break;
    }
}

unsigned
user_workspace_t::get_number() const
{
    return m_number;
}

const ::std::deque<client_ptr_t>&
user_workspace_t::get_all() const
{
    return m_clients.get_all();
}

bool
user_workspace_t::empty() const
{
    return m_clients.get_all().empty();
}

bool
user_workspace_t::contains(client_ptr_t client) const
{
    return m_clients.contains(client);
}

bool
user_workspace_t::is_mirrored() const
{
    return m_mirrored;
}

bool
user_workspace_t::in_float_layout() const
{
    return m_layout == layout_t::floating;
}

bool
user_workspace_t::in_monocle_layout() const
{
    return m_layout == layout_t::monocle;
}

const
client_ptr_t
user_workspace_t::get_focused() const
{
    return m_clients.get();
}

void
user_workspace_t::set_focused(client_ptr_t client, bool ignore_unwind)
{
    m_clients.set(client, ignore_unwind);
}

void
user_workspace_t::unset_focused()
{
    m_clients.unset();
}

user_workspace_t&
user_workspace_t::add_client(client_ptr_t client)
{
    m_clients.add(client);
    ::std::for_each(client->children.begin(), client->children.end(),
        [=](client_ptr_t child) { m_clients.add(child); });

    return *this;
}

user_workspace_t&
user_workspace_t::remove_client(client_ptr_t client)
{
    ::std::for_each(client->children.begin(), client->children.end(),
        [=](client_ptr_t child) { m_clients.remove(child); });
    m_clients.remove(client);

    return *this;
}

user_workspace_t&
user_workspace_t::forward()
{
    m_clients.next_focus();
    return *this;
}

user_workspace_t&
user_workspace_t::backward()
{
    m_clients.prev_focus();
    return *this;
}

user_workspace_t&
user_workspace_t::move_forward()
{
    if (!m_clients.get()->floating) {
        auto moved = m_clients.move_focus_forward();
        if (moved.first && moved.second) {
            if (in_float_layout()) {
                pos_t pos1 = moved.first->float_pos, pos2 = moved.second->float_pos;
                dim_t dim1 = moved.first->float_dim, dim2 = moved.second->float_dim;
                moved.first->resize(dim2).move(pos2);
                moved.second->resize(dim1).move(pos1);
            } else {
                arrange();
            }
        }
    }

    return *this;
}

user_workspace_t&
user_workspace_t::move_backward()
{
    if (!m_clients.get()->floating) {
        auto moved = m_clients.move_focus_backward();
        if (moved.first && moved.second) {
            if (in_float_layout()) {
                pos_t pos1 = moved.first->float_pos, pos2 = moved.second->float_pos;
                dim_t dim1 = moved.first->float_dim, dim2 = moved.second->float_dim;
                moved.first->resize(dim2).move(pos2);
                moved.second->resize(dim1).move(pos1);
            } else
                arrange();
        }
    }

    return *this;
}


user_workspace_t&
user_workspace_t::rotate_stack_forward()
{
    if (!in_monocle_layout() && m_clients.size() - m_nmaster > 1) {
        m_clients.rotate_group_forward(m_nmaster, m_clients.size());
        arrange();
    }

    return *this;
}

user_workspace_t&
user_workspace_t::rotate_stack_backward()
{
    if (!in_monocle_layout() && m_clients.size() - m_nmaster > 1) {
        m_clients.rotate_group_backward(m_nmaster, m_clients.size());
        arrange();
    }

    return *this;
}

user_workspace_t&
user_workspace_t::rotate_master_forward()
{
    if (!in_monocle_layout() && m_nmaster > 1) {
        m_clients.rotate_group_backward(0, m_nmaster);
        arrange();
    }

    return *this;
}

user_workspace_t&
user_workspace_t::rotate_master_backward()
{
    if (!in_monocle_layout() && m_nmaster > 1) {
        m_clients.rotate_group_forward(0, m_nmaster);
        arrange();
    }

    return *this;
}


user_workspace_t&
user_workspace_t::zoom()
{
    if (!empty())
    {
        auto zoomed = m_clients.zoom();
        if (in_float_layout() && zoomed.first && zoomed.second) {
            pos_t pos1 = zoomed.first->float_pos, pos2 = zoomed.second->float_pos;
            dim_t dim1 = zoomed.first->float_dim, dim2 = zoomed.second->float_dim;
            zoomed.first->resize(dim2).move(pos2);
            zoomed.second->resize(dim1).move(pos1);
        } else
            arrange();
    }

    return *this;
}

user_workspace_t&
user_workspace_t::mirror()
{
    m_mirrored = !m_mirrored;
    return *this;
}

user_workspace_t&
user_workspace_t::jump_pane()
{
    static client_ptr_t master_client, stack_client;
    unsigned i = m_clients.index_of(m_clients.get());

    if (!(m_nmaster == 0 || m_nmaster >= m_clients.size())) {
        if (master_client && m_clients.index_of(master_client) >= m_nmaster)
            master_client = nullptr;

        if (stack_client && m_clients.index_of(stack_client) < m_nmaster)
            stack_client = nullptr;

        if (i >= m_nmaster) {
            stack_client = m_clients.get();
            if (!m_clients.set(master_client))
                m_clients.set(static_cast<unsigned>(0u));
        } else {
            master_client = m_clients.get();
            if (!m_clients.set(stack_client))
                m_clients.set(m_nmaster);
        }
    }

    return *this;
}

user_workspace_t&
user_workspace_t::set_nmaster(unsigned new_nmaster)
{
    m_nmaster = new_nmaster;
    return *this;
}

user_workspace_t&
user_workspace_t::set_gap_size(unsigned new_gap_size)
{
    m_gap_size = new_gap_size;
    return *this;
}

user_workspace_t&
user_workspace_t::set_mfactor(float new_mfactor)
{
    m_mfactor = new_mfactor;
    return *this;
}

user_workspace_t&
user_workspace_t::set_m1weight(unsigned new_m1weight)
{
    m_m1weight = new_m1weight;
    return *this;
}

user_workspace_t&
user_workspace_t::set_layout(layout_t _layout)
{
    layout_t current_layout = m_layout;
    if (_layout == layout_t::toggle)
        m_layout = m_previous_layout;
    else
        m_layout = _layout;

    m_previous_layout = current_layout;
    return *this;
}


unsigned
user_workspace_t::get_nmaster() const
{
    return m_nmaster;
}

unsigned
user_workspace_t::get_gap_size() const
{
    return m_gap_size;
}

float
user_workspace_t::get_mfactor() const
{
    return m_mfactor;
}

unsigned
user_workspace_t::get_m1weight() const
{
    return m_m1weight;
}

layout_t
user_workspace_t::get_layout() const
{
    return m_layout;
}

bool
user_workspace_t::is_master(client_ptr_t client)
{
    unsigned index = m_clients.index_of(client);
    return index < m_nmaster;
}

bool
user_workspace_t::is_stack(client_ptr_t client)
{
    unsigned index = m_clients.index_of(client);
    return index >= m_nmaster && m_nmaster <= m_clients.size();
}

bool
user_workspace_t::master_focused()
{
    return is_master(m_clients.get());
}

bool
user_workspace_t::stack_focused()
{
    return is_stack(m_clients.get());
}
