#include "focus-cycle.hh"
#include "util.hh"

#include <algorithm>


void
focus_cycle::add(client_ptr_t client)
{
    if (m_has_focus)
        m_previously_focused_client = *m_focus;

    m_clients.push_back(client);
    m_focus = m_clients.end() - 1;
    m_has_focus = true;
}

void
focus_cycle::remove(client_ptr_t client)
{
    client_ptr_t prev_focus = *m_focus;
    erase_remove(m_clients, client);

    if (is_empty()) {
        m_has_focus = false;
        return;
    }

    if (prev_focus == client)
        if (m_focus >= m_clients.end())
            --m_focus;

    if (m_previously_focused_client)
        set(m_previously_focused_client);
}

bool
focus_cycle::is_empty() const
{
    return m_clients.empty();
}

::std::deque<client_ptr_t>::size_type
focus_cycle::size() const
{
    return m_clients.size();
}

bool
focus_cycle::contains(client_ptr_t c) const
{
    return ::std::find(m_clients.begin(), m_clients.end(), c)
        != m_clients.end();
}

const client_ptr_t
focus_cycle::get() const
{
    if (!m_has_focus)
        return nullptr;

    return *m_focus;
}

const ::std::deque<client_ptr_t>&
focus_cycle::get_all() const
{
    return m_clients;
}

bool
focus_cycle::set(client_ptr_t c)
{
    auto it = ::std::find(m_clients.begin(), m_clients.end(), c);
    if (it == m_clients.end())
        return false;

    m_previously_focused_client = *m_focus;
    m_focus = it;
    return true;
}

bool
focus_cycle::set(fg_sz index)
{
    if (index >= m_clients.size())
        return false;

    m_previously_focused_client = *m_focus;
    m_focus = m_clients.begin() + index;
    return true;
}

void
focus_cycle::unset()
{
    m_has_focus = false;
}

unsigned
focus_cycle::index_of(client_ptr_t client)
{
    fg_it it = ::std::find(m_clients.begin(), m_clients.end(), client);
    return it - m_clients.begin();
}

bool // moves down towards first (mod+k)
focus_cycle::next_focus()
{
    if (m_clients.size() <= 1)
        return false;

    if (m_focus == m_clients.begin())
        m_focus = m_clients.end() - 1;
    else
        --m_focus;

    return true;
}

bool // moves up towards last (mod+j)
focus_cycle::prev_focus()
{
    if (m_clients.size() <= 1)
        return false;

    if (++m_focus == m_clients.end())
        m_focus = m_clients.begin();

    return true;
}

void
focus_cycle::rotate_group_forward(unsigned from, unsigned to)
{
    if (from > to || from > m_clients.size() || to > m_clients.size())
        return;

    ::std::map<client_ptr_t, unsigned> floating_clients;
    unsigned index = 0;
    for (auto& client : get_all()) {
        if (client->floating)
            floating_clients[client] = index;
        ++index;
    }

    auto clients = get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t c) { return c->floating; }), clients.end());

    fg_it from_it = clients.begin() + from;
    fg_it to_it   = clients.begin() + to;

    if (from > 0)
        to_it -= m_clients.size() - clients.size();

    ::std::rotate(from_it, from_it + 1, to_it);

    for (auto&& [_client,_index] : floating_clients)
        clients.insert(clients.begin() + _index, _client);

    m_clients = clients;
}

void
focus_cycle::rotate_group_backward(unsigned from, unsigned to)
{
    if (from > to || from > m_clients.size() || to > m_clients.size())
        return;

    ::std::map<client_ptr_t, unsigned> floating_clients;
    unsigned index = 0;
    for (auto& client : get_all()) {
        if (client->floating)
            floating_clients[client] = index;
        ++index;
    }

    auto clients = get_all();
    clients.erase(::std::remove_if(clients.begin(), clients.end(),
        [](client_ptr_t c) { return c->floating; }), clients.end());

    fg_rit from_it = clients.rend() - to;
    fg_rit to_it   = clients.rend() - from;

    if (from > 0)
        from_it += m_clients.size() - clients.size();

    ::std::rotate(from_it, from_it + 1, to_it);

    for (auto&& [_client,_index] : floating_clients)
        clients.insert(clients.begin() + _index, _client);

    m_clients = clients;
}

::std::pair<client_ptr_t, client_ptr_t>
focus_cycle::move_focused_client_forward()
{
    if (m_clients.size() <= 1)
        return {nullptr, nullptr};

    auto to = m_focus;
    do {
        if (to == m_clients.begin())
            to = m_clients.end();
    } while ((*--to)->floating);

    if (to == m_focus)
        return {nullptr, nullptr};

    auto from = m_focus;
    ::std::iter_swap(from, to);
    m_focus = to;
    return {*from, *to};
}

::std::pair<client_ptr_t, client_ptr_t>
focus_cycle::move_focused_client_backward()
{
    if (m_clients.size() <= 1)
        return {nullptr, nullptr};

    auto to = m_focus;
    do {
        if (++to == m_clients.end())
            to = m_clients.begin();
    } while ((*to)->floating);

    if (to == m_focus)
        return {nullptr, nullptr};

    auto from = m_focus;
    ::std::iter_swap(from, to);
    m_focus = to;
    return {*from, *to};
}

::std::pair<client_ptr_t, client_ptr_t>
focus_cycle::zoom()
{
    static client_ptr_t prev_zoomed_out = nullptr;

    if (m_clients.size() <= 1)
        return {nullptr, nullptr};

    if (m_focus == m_clients.begin())
        set(prev_zoomed_out);

    prev_zoomed_out = m_clients.front();

    ::std::iter_swap(m_focus, m_clients.begin());
    client_ptr_t prev_focused = *m_focus;
    m_focus = m_clients.begin();
    return {prev_focused, *m_focus};
}
