#ifndef __CONTEXT_H_GUARD__
#define __CONTEXT_H_GUARD__

#include "../winsys/common.hh"
#include "cycle.hh"

#include <string>
#include <unordered_set>

typedef class Client* Client_ptr;
typedef class Workspace* Workspace_ptr;
typedef class Partition* Partition_ptr;

typedef class Context final
{
public:
    Context(Index index, std::string name)
        : m_index(index),
          m_name(name),
          mp_partition(nullptr),
          mp_active(nullptr),
          mp_prev_active(nullptr),
          m_workspaces({}, true)
    {}

    Index
    index() const
    {
        return m_index;
    }

    std::string const&
    name() const
    {
        return m_name;
    }

    std::size_t
    size() const
    {
        return m_workspaces.size();
    }

    Workspace_ptr
    workspace() const
    {
        return mp_active;
    }

    Workspace_ptr
    prev_workspace() const
    {
        return mp_prev_active;
    }

    Partition_ptr
    partition() const
    {
        return mp_partition;
    }

    bool
    is_partitioned() const
    {
        return mp_partition != nullptr;
    }

    void
    set_partition(Partition_ptr partition)
    {
        mp_partition = partition;
    }

    void
    register_workspace(Workspace_ptr workspace)
    {
        m_workspaces.insert_at_back(workspace);
    }

    void
    activate_workspace(Index index)
    {
        Workspace_ptr prev_active = mp_active;
        m_workspaces.activate_at_index(index);
        mp_active = *m_workspaces.active_element();

        if (prev_active != mp_active)
            mp_prev_active = prev_active;
    }

    void
    activate_workspace(Workspace_ptr workspace)
    {
        Workspace_ptr prev_active = mp_active;
        m_workspaces.activate_element(workspace);
        mp_active = workspace;

        if (prev_active != mp_active)
            mp_prev_active = prev_active;
    }

    Cycle<Workspace_ptr> const&
    workspaces() const
    {
        return m_workspaces;
    }

    void
    register_sticky_client(Client_ptr client)
    {
        m_sticky_clients.insert(client);
    }

    void
    unregister_sticky_client(Client_ptr client)
    {
        m_sticky_clients.erase(client);
    }

    std::unordered_set<Client_ptr> const&
    sticky_clients() const
    {
        return m_sticky_clients;
    }

    std::deque<Workspace_ptr>::iterator
    begin()
    {
        return m_workspaces.begin();
    }

    std::deque<Workspace_ptr>::const_iterator
    begin() const
    {
        return m_workspaces.begin();
    }

    std::deque<Workspace_ptr>::const_iterator
    cbegin() const
    {
        return m_workspaces.cbegin();
    }

    std::deque<Workspace_ptr>::iterator
    end()
    {
        return m_workspaces.end();
    }

    std::deque<Workspace_ptr>::const_iterator
    end() const
    {
        return m_workspaces.end();
    }

    std::deque<Workspace_ptr>::const_iterator
    cend() const
    {
        return m_workspaces.cend();
    }

    Workspace_ptr
    operator[](std::size_t i)
    {
        return m_workspaces[i];
    }

    Workspace_ptr
    operator[](std::size_t i) const
    {
        return m_workspaces[i];
    }

private:
    Index m_index;
    std::string m_name;

    Partition_ptr mp_partition;

    Workspace_ptr mp_active;
    Workspace_ptr mp_prev_active;

    Cycle<Workspace_ptr> m_workspaces;
    std::unordered_set<Client_ptr> m_sticky_clients;

}* Context_ptr;

#endif//__CONTEXT_H_GUARD__
