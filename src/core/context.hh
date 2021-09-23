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

    Partition_ptr
    partition() const
    {
        return mp_partition;
    }

    void
    register_workspace(Workspace_ptr workspace)
    {
        m_workspaces.insert_at_back(workspace);
    }

    void
    activate_workspace(Workspace_ptr workspace)
    {
        m_workspaces.activate_element(workspace);
    }

    void
    activate_workspace(Index index)
    {
        m_workspaces.activate_at_index(index);
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

private:
    Index m_index;
    std::string m_name;

    Partition_ptr mp_partition;

    Workspace_ptr mp_active;
    Cycle<Workspace_ptr> m_workspaces;
    std::unordered_set<Client_ptr> m_sticky_clients;

}* Context_ptr;

#endif//__CONTEXT_H_GUARD__
