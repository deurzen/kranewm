#ifndef __KRANEWM_CHANGES_GUARD__
#define __KRANEWM_CHANGES_GUARD__

#include "client.hh"
#include "workspace.hh"

#include <queue>


enum change
{
    CHANGE_NOOP = 0,
    CLIENT_FOCUS_CHANGE,
    CLIENT_DESTROY_CHANGE,
    CLIENT_WORKSPACE_CHANGE,
};


typedef class clientchange_t* clientchange_ptr_t;


class changequeue_t
{
public:
    bool has_next() const;
    clientchange_ptr_t next();

    void add(clientchange_ptr_t);
    void flush();

private:
    ::std::queue<clientchange_ptr_t> m_changes;

};


typedef struct clientchange_t
{
    explicit clientchange_t(change _type = CHANGE_NOOP)
      : type(_type) {}

    virtual ~clientchange_t() = default;

    change type;

}* clientchange_ptr_t;



typedef struct clientfocuschange_t : clientchange_t
{
    explicit clientfocuschange_t(client_ptr_t _from, client_ptr_t _to)
      : clientchange_t(CLIENT_FOCUS_CHANGE), from(_from), to(_to) {}

    client_ptr_t from;
    client_ptr_t to;

}* clientfocuschange_ptr_t;

inline clientfocuschange_ptr_t change_client_focus(client_ptr_t from, client_ptr_t to)
{
    return new clientfocuschange_t(from, to);
}

inline clientfocuschange_ptr_t change_client_focus(clientchange_ptr_t change)
{
    return dynamic_cast<clientfocuschange_ptr_t>(change);
}



typedef struct clientdestroychange_t : clientchange_t
{
    explicit clientdestroychange_t(client_ptr_t _client, workspace_ptr_t _workspace)
      : clientchange_t(CLIENT_DESTROY_CHANGE), client(_client), workspace(_workspace) {}

    client_ptr_t client;
    workspace_ptr_t workspace;

}* clientdestroychange_ptr_t;

inline clientdestroychange_ptr_t change_client_destroy(client_ptr_t client, workspace_ptr_t workspace)
{
    return new clientdestroychange_t(client, workspace);
}

inline clientdestroychange_ptr_t change_client_destroy(clientchange_ptr_t change)
{
    return dynamic_cast<clientdestroychange_ptr_t>(change);
}



typedef struct clientworkspacechange_t : clientchange_t
{
    explicit clientworkspacechange_t(client_ptr_t _client, workspace_ptr_t _from, workspace_ptr_t _to)
      : clientchange_t(CLIENT_WORKSPACE_CHANGE), client(_client), from(_from), to(_to) {}

    client_ptr_t client;
    workspace_ptr_t from;
    workspace_ptr_t to;

}* clientworkspacechange_ptr_t;

inline clientworkspacechange_ptr_t change_client_workspace(client_ptr_t client,
    workspace_ptr_t from, workspace_ptr_t to)
{
    return new clientworkspacechange_t(client, from, to);
}

inline clientworkspacechange_ptr_t change_client_workspace(clientchange_ptr_t change)
{
    return dynamic_cast<clientworkspacechange_ptr_t>(change);
}



#endif//__KRANEWM_CHANGES_GUARD__
