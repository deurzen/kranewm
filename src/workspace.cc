#include "workspace.hh"


void
user_workspace_t::arrange() const
{
    switch (m_layout) {
    case layout_t::floating:    m_layouthandler.layout_floating(*this);    break;
    case layout_t::tile:        m_layouthandler.layout_tile(*this);        break;
    case layout_t::stick:       m_layouthandler.layout_stick(*this);       break;
    case layout_t::deck:        m_layouthandler.layout_deck(*this);        break;
    case layout_t::doubledeck:  m_layouthandler.layout_doubledeck(*this);  break;
    case layout_t::sdeck:       m_layouthandler.layout_sdeck(*this);       break;
    case layout_t::sdoubledeck: m_layouthandler.layout_sdoubledeck(*this); break;
    case layout_t::grid:        m_layouthandler.layout_grid(*this);        break;
    case layout_t::pillar:      m_layouthandler.layout_pillar(*this);      break;
    case layout_t::column:      m_layouthandler.layout_column(*this);      break;
    case layout_t::monocle:     m_layouthandler.layout_monocle(*this);     break;
    case layout_t::center:      m_layouthandler.layout_center(*this);      break;
    case layout_t::centerstack: m_layouthandler.layout_centerstack(*this); break;
    default: break;
    }
}

::std::size_t
user_workspace_t::get_number() const
{
    return m_number;
}

::std::size_t
user_workspace_t::get_index() const
{
    return m_number - 1;
}

const ::std::deque<client_ptr_t>&
user_workspace_t::get_all() const
{
    return m_clients.get_all();
}

const ::std::deque<client_ptr_t>&
user_workspace_t::get_icons() const
{
    return m_icons.get_all();
}

const ::std::deque<client_ptr_t>&
user_workspace_t::get_disowned() const
{
    return m_disowned.get_all();
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
user_workspace_t::has_sidebar() const
{
    return m_sidebarset;
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
    m_stack.raise(client);

    for (auto& child : client->children)
        m_stack.raise(child);
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
    m_stack.add(client);

    return *this;
}

user_workspace_t&
user_workspace_t::remove_client(client_ptr_t client)
{
    m_clients.remove(client);
    m_stack.remove(client);

    return *this;
}

user_workspace_t&
user_workspace_t::add_family(client_ptr_t client)
{
    add_client(client);

    for (auto& child : client->children)
        add_client(child);

    return *this;
}

user_workspace_t&
user_workspace_t::remove_family(client_ptr_t client)
{
    for (auto& child : client->children)
        remove_client(child);

    remove_client(client);

    return *this;
}

user_workspace_t&
user_workspace_t::add_icon(client_ptr_t client)
{
    m_icons.add(client);
    return *this;
}

user_workspace_t&
user_workspace_t::remove_icon(client_ptr_t client)
{
    m_icons.remove(client);
    return *this;
}

user_workspace_t&
user_workspace_t::add_disowned(client_ptr_t client)
{
    m_disowned.add(client);
    m_stack.add(client);
    return *this;
}

user_workspace_t&
user_workspace_t::remove_disowned(client_ptr_t client)
{
    m_disowned.remove(client);
    m_stack.remove(client);
    return *this;
}

user_workspace_t&
user_workspace_t::raise_client(client_ptr_t client)
{
    m_stack.raise(client);
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
                moved.first->moveresize(pos2, dim2);
                moved.second->moveresize(pos1, dim1);
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
                moved.first->moveresize(pos2, dim2);
                moved.second->moveresize(pos1, dim1);
            } else
                arrange();
        }
    }

    return *this;
}


user_workspace_t&
user_workspace_t::rotate_clients_forward()
{
    if (!in_monocle_layout() && m_clients.size()) {
        m_clients.rotate_group_forward(0, m_clients.size());
        arrange();
    }

    return *this;
}

user_workspace_t&
user_workspace_t::rotate_clients_backward()
{
    if (!in_monocle_layout() && m_clients.size()) {
        m_clients.rotate_group_backward(0, m_clients.size());
        arrange();
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
            zoomed.first->moveresize(pos2, dim2);
            zoomed.second->moveresize(pos1, dim1);
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
user_workspace_t::sidebarset(bool sidebarset)
{
    m_sidebarset = sidebarset;
    return *this;
}

user_workspace_t&
user_workspace_t::jump_pane()
{
    static client_ptr_t master_client, stack_client;
    ::std::size_t i = m_clients.index_of(m_clients.get());

    if (!(m_nmaster == 0 || m_nmaster >= m_clients.size())) {
        if (master_client && m_clients.index_of(master_client) >= m_nmaster)
            master_client = nullptr;

        if (stack_client && m_clients.index_of(stack_client) < m_nmaster)
            stack_client = nullptr;

        if (i >= m_nmaster) {
            stack_client = m_clients.get();
            if (!m_clients.set(master_client))
                m_clients.set(static_cast<::std::size_t>(0u));
        } else {
            master_client = m_clients.get();
            if (!m_clients.set(stack_client))
                m_clients.set(m_nmaster);
        }
    }

    return *this;
}

user_workspace_t&
user_workspace_t::set_nmaster(::std::size_t new_nmaster)
{
    m_nmaster = new_nmaster;
    return *this;
}

user_workspace_t&
user_workspace_t::set_gapsize(::std::size_t new_gapsize)
{
    m_gapsize = new_gapsize;
    return *this;
}

user_workspace_t&
user_workspace_t::set_mfactor(float new_mfactor)
{
    m_mfactor = new_mfactor;
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

    if (_layout != current_layout)
        m_previous_layout = current_layout;

    return *this;
}


::std::size_t
user_workspace_t::get_nmaster() const
{
    return m_nmaster;
}

::std::size_t
user_workspace_t::get_gapsize() const
{
    return m_gapsize;
}

float
user_workspace_t::get_mfactor() const
{
    return m_mfactor;
}

layout_t
user_workspace_t::get_layout() const
{
    return m_layout;
}

const workspacestack_t&
user_workspace_t::get_stack() const
{
    return m_stack;
}

::std::size_t
user_workspace_t::get_nurgent() const
{
    return m_nurgent;
}

bool
user_workspace_t::is_urgent() const
{
    return m_nurgent > 0;
}

void
user_workspace_t::record_urgent()
{
    ++m_nurgent;
}

void
user_workspace_t::erase_urgent()
{
    if (m_nurgent)
        --m_nurgent;
}

bool
user_workspace_t::is_master(client_ptr_t client)
{
    ::std::size_t index = m_clients.index_of(client);
    return index < m_nmaster;
}

bool
user_workspace_t::is_stack(client_ptr_t client)
{
    ::std::size_t index = m_clients.index_of(client);
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

bool
user_workspace_t::reset()
{
    if (is_mirrored() != DEFAULT_MIRRORED)
        mirror();

    set_nmaster(DEFAULT_NMASTER);
    set_gapsize(DEFAULT_GAPSIZE);
    set_mfactor(DEFAULT_MFACTOR);
    set_layout(DEFAULT_LAYOUT);

    return m_sidebarset != DEFAULT_SIDEBARSET;
}
