#ifndef __KRANEWM_CHANGES_GUARD__
#define __KRANEWM_CHANGES_GUARD__

#include "client.hh"
#include "workspace.hh"

#include <queue>


enum change
{
    CHANGE_NOOP = 0,
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

typedef struct clientworkspacechange_t : clientchange_t
{
    explicit clientworkspacechange_t(client_ptr_t _client, workspace_ptr_t _from, workspace_ptr_t _to)
      : clientchange_t(CLIENT_WORKSPACE_CHANGE), client(_client), from(_from), to(_to) {}

    client_ptr_t client;
    workspace_ptr_t from;
    workspace_ptr_t to;

}* clientchangeworkspace_ptr_t;


#endif//__KRANEWM_CHANGES_GUARD__
