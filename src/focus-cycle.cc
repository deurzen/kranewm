#include "focus-cycle.hh"
#include "util.hh"

#include <algorithm>


void
focus_cycle::add(client_ptr_t client)
{
    if (has_focus_)
        previously_focused_client_ = *focus_;

    clients_.push_back(client);
    focus_ = clients_.end() - 1;
    has_focus_ = true;
}

void
focus_cycle::remove(client_ptr_t client)
{
    client_ptr_t prev_focus = *focus_;
    erase_remove(clients_, client);

    if (is_empty()) {
        has_focus_ = false;
        return;
    }

    if (prev_focus == client)
        if (focus_ >= clients_.end())
            --focus_;

    if (previously_focused_client_)
        set(previously_focused_client_);
}

bool
focus_cycle::is_empty() const
{
    return clients_.empty();
}

::std::deque<client_ptr_t>::size_type
focus_cycle::size() const
{
    return clients_.size();
}

bool
focus_cycle::contains(client_ptr_t c) const
{
    return ::std::find(clients_.begin(), clients_.end(), c)
        != clients_.end();
}

const client_ptr_t
focus_cycle::get() const
{
    if (!has_focus_)
        return nullptr;

    return *focus_;
}

const ::std::deque<client_ptr_t>&
focus_cycle::get_all() const
{
    return clients_;
}

bool
focus_cycle::set(client_ptr_t c)
{
    auto it = ::std::find(clients_.begin(), clients_.end(), c);
    if (it == clients_.end())
        return false;

    focus_ = it;
    return true;
}

bool
focus_cycle::set(fg_sz index)
{
    if (index >= clients_.size())
        return false;

    focus_ = clients_.begin() + index;
    return true;
}

void
focus_cycle::unset()
{
    has_focus_ = false;
}

unsigned
focus_cycle::index_of(client_ptr_t client)
{
    fg_it it = ::std::find(clients_.begin(), clients_.end(), client);
    return it - clients_.begin();
}

bool // moves down towards first (mod+k)
focus_cycle::next_focus()
{
    if (clients_.size() <= 1)
        return false;

    if (focus_ == clients_.begin())
        focus_ = clients_.end() - 1;
    else
        --focus_;

    return true;
}

bool // moves up towards last (mod+j)
focus_cycle::prev_focus()
{
    if (clients_.size() <= 1)
        return false;

    if (++focus_ == clients_.end())
        focus_ = clients_.begin();

    return true;
}

void
focus_cycle::rotate_group_forward(unsigned from, unsigned to)
{
    if (from > to || from > clients_.size() || to > clients_.size())
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
        to_it -= clients_.size() - clients.size();

    ::std::rotate(from_it, from_it + 1, to_it);

    for (auto&& [_client,_index] : floating_clients)
        clients.insert(clients.begin() + _index, _client);

    clients_ = clients;
}

void
focus_cycle::rotate_group_backward(unsigned from, unsigned to)
{
    if (from > to || from > clients_.size() || to > clients_.size())
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
        from_it += clients_.size() - clients.size();

    ::std::rotate(from_it, from_it + 1, to_it);

    for (auto&& [_client,_index] : floating_clients)
        clients.insert(clients.begin() + _index, _client);

    clients_ = clients;
}

::std::pair<client_ptr_t, client_ptr_t>
focus_cycle::move_focused_client_forward()
{
    if (clients_.size() <= 1)
        return {nullptr, nullptr};

    auto to = focus_;
    do {
        if (to == clients_.begin())
            to = clients_.end();
    } while ((*--to)->floating);

    if (to == focus_)
        return {nullptr, nullptr};

    auto from = focus_;
    ::std::iter_swap(from, to);
    focus_ = to;
    return {*from, *to};
}

::std::pair<client_ptr_t, client_ptr_t>
focus_cycle::move_focused_client_backward()
{
    if (clients_.size() <= 1)
        return {nullptr, nullptr};

    auto to = focus_;
    do {
        if (++to == clients_.end())
            to = clients_.begin();
    } while ((*to)->floating);

    if (to == focus_)
        return {nullptr, nullptr};

    auto from = focus_;
    ::std::iter_swap(from, to);
    focus_ = to;
    return {*from, *to};
}

::std::pair<client_ptr_t, client_ptr_t>
focus_cycle::zoom()
{
    static client_ptr_t prev_zoomed_out = nullptr;

    if (clients_.size() <= 1)
        return {nullptr, nullptr};

    if (focus_ == clients_.begin())
        set(prev_zoomed_out);

    prev_zoomed_out = clients_.front();

    ::std::iter_swap(focus_, clients_.begin());
    client_ptr_t prev_focused = *focus_;
    focus_ = clients_.begin();
    return {prev_focused, *focus_};
}
