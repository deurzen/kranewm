#include "process.hh"

#include "client.hh"


void
processjumplist_t::activate_process_name(const ::std::string& name)
{
    if (m_jumps.find(name) != m_jumps.end())
        return;

    m_jumps[name] = {};
}

void
processjumplist_t::deactivate_process_name(const ::std::string& name)
{
    if (m_jumps.find(name) == m_jumps.end())
        return;

    m_jumps.erase(name);
}


void
processjumplist_t::add_process(client_ptr_t client)
{
    if (m_jumps.find(client->name) == m_jumps.end())
        return;

    m_jumps[client->name].push_back(new processjump_t{client, nullptr});
}

void
processjumplist_t::remove_process(client_ptr_t client)
{
    if (m_jumps.find(client->name) == m_jumps.end())
        return;

    erase_remove(m_jumps[client->name], find_process(client));
}

void
processjumplist_t::relayer_process(client_ptr_t client)
{
    if (m_jumps.find(client->name) == m_jumps.end())
        return;

    auto it = ::std::find_if(m_jumps[client->name].begin(), m_jumps[client->name].end(),
        [client](processjump_ptr_t process) { return process->target == client; });

    if (it != m_jumps[client->name].end() && m_jumps[client->name].size() > 1)
        ::std::iter_swap(it, m_jumps[client->name].end() - 1);
}


processjump_ptr_t
processjumplist_t::find_process(client_ptr_t client)
{
    if (m_jumps.find(client->name) == m_jumps.end())
        return nullptr;

    auto it = ::std::find_if(m_jumps[client->name].begin(), m_jumps[client->name].end(),
        [client](processjump_ptr_t process) { return process->target == client; });

    if (it != m_jumps[client->name].end())
        return *it;
    else
        return nullptr;
}

processjump_ptr_t
processjumplist_t::get_process(const ::std::string& name)
{
    auto it = m_jumps.find(name);
    if (it == m_jumps.end())
        return nullptr;

    if (!it->second.empty())
        return it->second.back();

    return nullptr;
}
