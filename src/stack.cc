#include "stack.hh"
#include "util.hh"
#include "client.hh"
#include "workspace.hh"

#include "x-data/attributes.hh"


void
workspacestack_t::add(client_ptr_t client)
{
    m_clients.push_back(client);
}

void
workspacestack_t::remove(client_ptr_t client)
{
    erase_remove(m_clients, client);
}

void
workspacestack_t::raise(client_ptr_t client)
{
    remove(client);
    add(client);
}

::std::vector<client_ptr_t>&
workspacestack_t::get_clients()
{
    return m_clients;
}


windowstack_t&
windowstack_t::add_to_stack(windowstack_window_t win)
{
    if (m_win_layers.count(win.win))
        return *this;

    switch (win.layer) {
    case layer_t::desktop:      m_desktop_windows.push_back(win.win);      break;
    case layer_t::below:        m_below_windows.push_back(win.win);        break;
    case layer_t::dock:         m_dock_windows.push_back(win.win);         break;
    case layer_t::above:        m_above_windows.push_back(win.win);        break;
    case layer_t::notification: m_notification_windows.push_back(win.win); break;
    }

    m_win_layers[win.win] = win.layer;
    return *this;
}

windowstack_t&
windowstack_t::remove_from_stack(x_data::window_t win)
{
    if (!m_win_layers.count(win))
        return *this;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      erase_find(m_desktop_windows, win);      break;
    case layer_t::below:        erase_find(m_below_windows, win);        break;
    case layer_t::dock:         erase_find(m_dock_windows, win);         break;
    case layer_t::above:        erase_find(m_above_windows, win);        break;
    case layer_t::notification: erase_find(m_notification_windows, win); break;
    }

    erase_find(m_win_layers, win);

    return *this;
}

windowstack_t&
windowstack_t::relayer_window(windowstack_window_t win)
{
    remove_from_stack(win.win);
    add_to_stack(win);
    return *this;
}

windowstack_t&
windowstack_t::raise_window(x_data::window_t win)
{
    if (!m_win_layers.count(win))
        return *this;

    ::std::list<x_data::window_t>* layer_list;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      layer_list = &m_desktop_windows;      break;
    case layer_t::below:        layer_list = &m_below_windows;        break;
    case layer_t::dock:         layer_list = &m_dock_windows;         break;
    case layer_t::above:        layer_list = &m_above_windows;        break;
    case layer_t::notification: layer_list = &m_notification_windows; break;
    default: return *this;
    }

    layer_list->remove(win);
    layer_list->push_front(win);

    return *this;
}

windowstack_t&
windowstack_t::lower_window(x_data::window_t win)
{
    if (!m_win_layers.count(win))
        return *this;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      splice_back(m_desktop_windows, win);      break;
    case layer_t::below:        splice_back(m_below_windows, win);        break;
    case layer_t::dock:         splice_back(m_dock_windows, win);         break;
    case layer_t::above:        splice_back(m_above_windows, win);        break;
    case layer_t::notification: splice_back(m_notification_windows, win); break;
    }

    return *this;
}

windowstack_t&
windowstack_t::raise_window_above(x_data::window_t win, x_data::window_t sibling)
{
    if (!m_win_layers.count(win) || !m_win_layers.count(sibling)
        || m_win_layers.at(win) != m_win_layers.at(sibling))
    {
        return *this;
    }

    ::std::list<x_data::window_t>* layer_list;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      layer_list = &m_desktop_windows;      break;
    case layer_t::below:        layer_list = &m_below_windows;        break;
    case layer_t::dock:         layer_list = &m_dock_windows;         break;
    case layer_t::above:        layer_list = &m_above_windows;        break;
    case layer_t::notification: layer_list = &m_notification_windows; break;
    default: return *this;
    }

    layer_list->remove(win);
    auto it = ::std::find(layer_list->begin(), layer_list->end(), sibling);
    layer_list->insert((it != layer_list->end()) ? ++it : it, win);

    return *this;
}

windowstack_t&
windowstack_t::lower_window_below(x_data::window_t win, x_data::window_t sibling)
{
    if (!m_win_layers.count(win) || !m_win_layers.count(sibling)
        || m_win_layers.at(win) != m_win_layers.at(sibling))
    {
        return *this;
    }

    ::std::list<x_data::window_t>* layer_list;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      layer_list = &m_desktop_windows;      break;
    case layer_t::below:        layer_list = &m_below_windows;        break;
    case layer_t::dock:         layer_list = &m_dock_windows;         break;
    case layer_t::above:        layer_list = &m_above_windows;        break;
    case layer_t::notification: layer_list = &m_notification_windows; break;
    default: return *this;
    }

    layer_list->remove(win);
    auto it = ::std::find(layer_list->begin(), layer_list->end(), sibling);
    layer_list->insert(it, sibling);

    return *this;
}

bool
windowstack_t::occluded_by(x_data::window_t win, x_data::window_t sibling)
{
    if (!m_win_layers.count(win) || !m_win_layers.count(sibling)
        || m_win_layers.at(win) != m_win_layers.at(sibling))
    {
        return false;
    }

    x_data::attributes_t win_attrs = x_data::get_attributes(win);
    x_data::attributes_t sibling_attrs = x_data::get_attributes(sibling);

    const pos_t win_topleft = win_attrs.pos();
    const pos_t win_bottomright = win_attrs.pos() + win_attrs.dim();

    const pos_t sibling_topleft = sibling_attrs.pos();
    const pos_t sibling_bottomright = sibling_attrs.pos() + sibling_attrs.dim();

    return !(win_topleft.x > sibling_bottomright.x
        || win_bottomright.x > sibling_topleft.x
        || win_topleft.y < sibling_bottomright.y
        || win_bottomright.y < sibling_topleft.y);
}

bool
windowstack_t::occluded_by_any(x_data::window_t win)
{
    if (!m_win_layers.count(win))
        return false;

    ::std::list<x_data::window_t>* layer_list;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      layer_list = &m_desktop_windows;      break;
    case layer_t::below:        layer_list = &m_below_windows;        break;
    case layer_t::dock:         layer_list = &m_dock_windows;         break;
    case layer_t::above:        layer_list = &m_above_windows;        break;
    case layer_t::notification: layer_list = &m_notification_windows; break;
    default: return false;
    }

    for (auto& sibling : *layer_list)
        if (win != sibling && occluded_by(win, sibling))
            return true;

    return false;
}

bool
windowstack_t::occludes_any(x_data::window_t win)
{
    if (!m_win_layers.count(win))
        return false;

    ::std::list<x_data::window_t>* layer_list;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      layer_list = &m_desktop_windows;      break;
    case layer_t::below:        layer_list = &m_below_windows;        break;
    case layer_t::dock:         layer_list = &m_dock_windows;         break;
    case layer_t::above:        layer_list = &m_above_windows;        break;
    case layer_t::notification: layer_list = &m_notification_windows; break;
    default: return false;
    }

    for (auto& sibling : *layer_list)
        if (win != sibling && occluded_by(sibling, win))
            return true;

    return false;
}


::std::list<x_data::window_t>
windowstack_t::get_all_of_type(layer_t type)
{
    switch (type) {
    case layer_t::desktop:      return m_desktop_windows;
    case layer_t::below:        return m_below_windows;
    case layer_t::dock:         return m_dock_windows;
    case layer_t::above:        return m_above_windows;
    case layer_t::notification: return m_notification_windows;
    default: return m_notification_windows;
    }
}

void
windowstack_t::apply(user_workspace_ptr_t workspace)
{
    apply(workspace->get_stack(), workspace->in_float_layout());
}

void
windowstack_t::apply(workspacestack_t stack, bool ignore_floating)
{
    auto workspace_clients = stack.get_clients();
    ::std::size_t n = m_win_layers.size() + workspace_clients.size();

    ::std::vector<Window> wins;
    wins.reserve(n);

    ::std::list<x_data::window_t> disowned_windows;
    ::std::list<x_data::window_t> above_windows;
    ::std::list<x_data::window_t> fullscreen_windows;
    ::std::list<x_data::window_t> floating_windows;
    ::std::list<x_data::window_t> normal_windows;
    ::std::list<x_data::window_t> below_windows;

    for (auto&& client : reverse(workspace_clients))
        if (client->disowned)
            disowned_windows.push_back(client->frame);
        else if (client->above)
            above_windows.push_back(client->frame);
        else if (!client->in_window && client->fullscreen)
            fullscreen_windows.push_back(client->frame);
        else if (client->below)
            below_windows.push_back(client->frame);
        else if (client->floating && !ignore_floating)
            floating_windows.push_back(client->frame);
        else
            normal_windows.push_back(client->frame);

    insert_container(wins, m_notification_windows);
    insert_container(wins, fullscreen_windows);
    insert_container(wins, m_above_windows);
    insert_container(wins, above_windows);
    insert_container(wins, floating_windows);
    insert_container(wins, disowned_windows);
    insert_container(wins, normal_windows);
    insert_container(wins, m_dock_windows);
    insert_container(wins, below_windows);
    insert_container(wins, m_below_windows);
    insert_container(wins, m_desktop_windows);

    x_data::restack_windows(wins.data(), n);
}
