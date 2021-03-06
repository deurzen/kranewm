#ifndef __KRANEWM_CHANGES_GUARD__
#define __KRANEWM_CHANGES_GUARD__

#include "client.hh"

#include <queue>

// fwd decls
typedef class context_t* context_ptr_t;
typedef class workspace_t* workspace_ptr_t;
typedef class user_workspace_t* user_workspace_ptr_t;


enum class change_t
{
    noop,
    client_focus,
    client_destroy,
    client_fullscreen,
    client_above,
    client_below,
    client_urgent,
    client_iconify,
    client_disown,
    client_sticky,
    client_workspace,
    client_context,
    workspace_active,
    context_active,
};


typedef class clientchange_t* clientchange_ptr_t;


class changequeue_t final
{
public:
    ~changequeue_t()
    {
        flush();
    }

    bool has_next() const;
    clientchange_ptr_t next();

    void add(clientchange_ptr_t);
    void flush();

private:
    ::std::queue<clientchange_ptr_t> m_changes;

};


typedef struct clientchange_t
{
    explicit clientchange_t(change_t _type = change_t::noop)
      : type(_type)
    {}

    virtual ~clientchange_t() = default;

    change_t type;

}* clientchange_ptr_t;



typedef struct clientfocuschange_t final : clientchange_t
{
    explicit clientfocuschange_t(client_ptr_t _from, client_ptr_t _to)
      : clientchange_t(change_t::client_focus),
        from(_from),
        to(_to)
    {}

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



typedef struct clientdestroychange_t final : clientchange_t
{
    explicit clientdestroychange_t(client_ptr_t _client, workspace_ptr_t _workspace)
      : clientchange_t(change_t::client_destroy),
        client(_client),
        workspace(_workspace)
    {}

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



typedef struct clientfullscreenchange_t final : clientchange_t
{
    explicit clientfullscreenchange_t(client_ptr_t _client, client_t _former_state)
      : clientchange_t(change_t::client_fullscreen),
        client(_client),
        former_state(_former_state)
    {}

    client_ptr_t client;
    client_t former_state;

}* clientfullscreenchange_ptr_t;

inline clientfullscreenchange_ptr_t change_client_fullscreen(client_ptr_t client, client_t former_state)
{
    return new clientfullscreenchange_t(client, former_state);
}

inline clientfullscreenchange_ptr_t change_client_fullscreen(clientchange_ptr_t change)
{
    return dynamic_cast<clientfullscreenchange_ptr_t>(change);
}



typedef struct clientabovechange_t final : clientchange_t
{
    explicit clientabovechange_t(client_ptr_t _client, client_t _former_state)
      : clientchange_t(change_t::client_above),
        client(_client),
        former_state(_former_state)
    {}

    client_ptr_t client;
    client_t former_state;

}* clientabovechange_ptr_t;

inline clientabovechange_ptr_t change_client_above(client_ptr_t client, client_t former_state)
{
    return new clientabovechange_t(client, former_state);
}

inline clientabovechange_ptr_t change_client_above(clientchange_ptr_t change)
{
    return dynamic_cast<clientabovechange_ptr_t>(change);
}



typedef struct clientbelowchange_t final : clientchange_t
{
    explicit clientbelowchange_t(client_ptr_t _client, client_t _former_state)
      : clientchange_t(change_t::client_below),
        client(_client),
        former_state(_former_state)
    {}

    client_ptr_t client;
    client_t former_state;

}* clientbelowchange_ptr_t;

inline clientbelowchange_ptr_t change_client_below(client_ptr_t client, client_t former_state)
{
    return new clientbelowchange_t(client, former_state);
}

inline clientbelowchange_ptr_t change_client_below(clientchange_ptr_t change)
{
    return dynamic_cast<clientbelowchange_ptr_t>(change);
}



typedef struct clienturgentchange_t final : clientchange_t
{
    explicit clienturgentchange_t(client_ptr_t _client)
      : clientchange_t(change_t::client_urgent),
        client(_client)
    {}

    client_ptr_t client;

}* clienturgentchange_ptr_t;

inline clienturgentchange_ptr_t change_client_urgent(client_ptr_t client)
{
    return new clienturgentchange_t(client);
}

inline clienturgentchange_ptr_t change_client_urgent(clientchange_ptr_t change)
{
    return dynamic_cast<clienturgentchange_ptr_t>(change);
}



typedef struct clienticonifychange_t final : clientchange_t
{
    explicit clienticonifychange_t(client_ptr_t _client)
      : clientchange_t(change_t::client_iconify),
        client(_client)
    {}

    client_ptr_t client;

}* clienticonifychange_ptr_t;

inline clienticonifychange_ptr_t change_client_iconify(client_ptr_t client)
{
    return new clienticonifychange_t(client);
}

inline clienticonifychange_ptr_t change_client_iconify(clientchange_ptr_t change)
{
    return dynamic_cast<clienticonifychange_ptr_t>(change);
}



typedef struct clientdisownchange_t final : clientchange_t
{
    explicit clientdisownchange_t(client_ptr_t _client, client_t _former_state)
      : clientchange_t(change_t::client_disown),
        client(_client),
        former_state(_former_state)
    {}

    client_ptr_t client;
    client_t former_state;

}* clientdisownchange_ptr_t;

inline clientdisownchange_ptr_t change_client_disown(client_ptr_t client, client_t former_state)
{
    return new clientdisownchange_t(client, former_state);
}

inline clientdisownchange_ptr_t change_client_disown(clientchange_ptr_t change)
{
    return dynamic_cast<clientdisownchange_ptr_t>(change);
}



typedef struct clientstickychange_t final : clientchange_t
{
    explicit clientstickychange_t(client_ptr_t _client, context_ptr_t _context)
      : clientchange_t(change_t::client_sticky),
        client(_client),
        context(_context)
    {}

    client_ptr_t client;
    context_ptr_t context;

}* clientstickychange_ptr_t;

inline clientstickychange_ptr_t change_client_sticky(client_ptr_t client, context_ptr_t context)
{
    return new clientstickychange_t(client, context);
}

inline clientstickychange_ptr_t change_client_sticky(clientchange_ptr_t change)
{
    return dynamic_cast<clientstickychange_ptr_t>(change);
}



typedef struct clientworkspacechange_t final : clientchange_t
{
    explicit clientworkspacechange_t(client_ptr_t _client, workspace_ptr_t _from, workspace_ptr_t _to)
      : clientchange_t(change_t::client_workspace),
        client(_client),
        from(_from),
        to(_to)
    {}

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



typedef struct clientcontextchange_t final : clientchange_t
{
    explicit clientcontextchange_t(client_ptr_t _client, context_ptr_t _from, context_ptr_t _to,
        user_workspace_ptr_t _from_workspace, user_workspace_ptr_t _to_workspace)
      : clientchange_t(change_t::client_context),
        client(_client),
        from(_from),
        to(_to),
        from_workspace(_from_workspace),
        to_workspace(_to_workspace)
    {}

    client_ptr_t client;
    context_ptr_t from;
    context_ptr_t to;
    user_workspace_ptr_t from_workspace;
    user_workspace_ptr_t to_workspace;

}* clientcontextchange_ptr_t;

inline clientcontextchange_ptr_t change_client_context(client_ptr_t client,
    context_ptr_t from, context_ptr_t to,
    user_workspace_ptr_t from_workspace, user_workspace_ptr_t to_workspace)
{
    return new clientcontextchange_t(client, from, to, from_workspace, to_workspace);
}

inline clientcontextchange_ptr_t change_client_context(clientchange_ptr_t change)
{
    return dynamic_cast<clientcontextchange_ptr_t>(change);
}



typedef struct workspaceactivechange_t final : clientchange_t
{
    explicit workspaceactivechange_t(user_workspace_ptr_t _from, user_workspace_ptr_t _to)
      : clientchange_t(change_t::workspace_active),
        from(_from),
        to(_to)
    {}

    user_workspace_ptr_t from;
    user_workspace_ptr_t to;

}* workspaceactivechange_ptr_t;

inline workspaceactivechange_ptr_t change_workspace_active(user_workspace_ptr_t from, user_workspace_ptr_t to)
{
    return new workspaceactivechange_t(from, to);
}

inline workspaceactivechange_ptr_t change_workspace_active(clientchange_ptr_t change)
{
    return dynamic_cast<workspaceactivechange_ptr_t>(change);
}



typedef struct contextactivechange_t final : clientchange_t
{
    explicit contextactivechange_t(context_ptr_t _from, context_ptr_t _to)
      : clientchange_t(change_t::context_active),
        from(_from),
        to(_to)
    {}

    context_ptr_t from;
    context_ptr_t to;

}* contextactivechange_ptr_t;

inline contextactivechange_ptr_t change_context_active(context_ptr_t from, context_ptr_t to)
{
    return new contextactivechange_t(from, to);
}

inline contextactivechange_ptr_t change_context_active(clientchange_ptr_t change)
{
    return dynamic_cast<contextactivechange_ptr_t>(change);
}

#endif//__KRANEWM_CHANGES_GUARD__
