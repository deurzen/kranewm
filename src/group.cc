#include "group.hh"
#include "client.hh"

#include "util.hh"


void
group_t::add_client(client_ptr_t client)
{
    m_clients.push_back(client);
}

void
group_t::remove_client(client_ptr_t client)
{
    erase_remove(m_clients, client);
}

void
group_t::diffuse_event(XKeyEvent* event) const
{
    for (auto client : m_clients) {
        event->window = client->win;
        XSendEvent(x_data::g_dpy, client->win, True, NoEventMask, (XEvent*)event);
    }
}

void
grouphandler_t::group_client(client_ptr_t client, size_t group_nr)
{
    if (!range_t<size_t>::contains(0, m_groups.size() - 1, group_nr))
        return;

    auto group = m_groups.at(group_nr);
    m_clientgroups[client] = group;
    group->add_client(client);
}

void
grouphandler_t::degroup_client(client_ptr_t client)
{
    if (!m_clientgroups.count(client))
        return;

    auto group = m_clientgroups.at(client);
    group->remove_client(client);
    erase_find(m_clientgroups, client);
}

bool
grouphandler_t::is_grouped(client_ptr_t client) const
{
    return m_clientgroups.count(client);
}
