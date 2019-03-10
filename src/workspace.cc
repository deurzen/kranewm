#include "workspace.hh"


void
user_workspace_t::arrange() const
{

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
