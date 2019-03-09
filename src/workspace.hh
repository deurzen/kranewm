#ifndef __KRANEWM_WORKSPACE_GUARD__
#define __KRANEWM_WORKSPACE_GUARD__

#include "client.hh"
#include "focus-cycle.hh"

#include <map>
#include <string>


const ::std::map<unsigned, ::std::string> USER_WORKSPACES({
    // nr   name
    { 1,   "1:main" },
    { 2,   "2:web"  },
    { 3,   "3:term" },
    { 4,   "4"      },
    { 5,   "5"      },
    { 6,   "6"      },
    { 7,   "7"      },
    { 8,   "8"      },
    { 9,   "9"      },
});


enum workspacetype
{
    WORKSPACE = 0,
    MOVE_WORKSPACE,
    RESIZE_WORKSPACE,
    USER_WORKSPACE,
};

enum layouttype
{
    LT_TOGGLE = 0,
    LT_FLOAT,
    LT_TILE,
    LT_DECK,
    LT_DOUBLEDECK,
    LT_GRID,
    LT_MONOCLE
};


typedef class workspace_t
{
public:
    explicit workspace_t(workspacetype _type = WORKSPACE)
        : type(_type) {}

    virtual ~workspace_t() {}

    virtual workspacetype get_type() const { return type; }

    virtual void arrange() const {}

    virtual workspace_t& add_client(client_ptr_t)    { return *this; }
    virtual workspace_t& remove_client(client_ptr_t) { return *this; }

private:
    workspacetype type;

}* workspace_ptr_t;

typedef class moveresize_workspace_t : public workspace_t
{
public:
    explicit moveresize_workspace_t(workspacetype _type)
        : workspace_t(_type),
          client(nullptr)
    {}

    void arrange() const override {}

    inline void set(client_ptr_t _client) { client = _client; }
    inline void unset() { client = nullptr; }

    inline client_ptr_t get() const { return client; }
    inline bool is_set() const { return client != nullptr; }

    inline moveresize_workspace_t& add_client(client_ptr_t client) override
    {
        if (!is_set())
            set(client);

        return *this;
    }

    inline moveresize_workspace_t& remove_client(client_ptr_t client) override
    {
        if (is_set() && client == get()) {
            unset();
        }

        return *this;
    }

private:
    client_ptr_t client;

}* moveresize_workspace_ptr_t;


inline bool is_move_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == MOVE_WORKSPACE;
}

inline bool is_resize_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == RESIZE_WORKSPACE;
}

inline bool is_moveresize_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == MOVE_WORKSPACE
        || workspace->get_type() == RESIZE_WORKSPACE;
}


typedef class user_workspace_t : public workspace_t
{
public:
    user_workspace_t(unsigned _number, ::std::string&& _name)
        : workspace_t(USER_WORKSPACE), number(_number), name(_name),
          n_master(1), gap_size(5), m1_weight(1), m_factor(.6f),
          mirrored(false), layout(LT_FLOAT), previous_layout(layout)
    {}

    inline unsigned get_number() const { return number; }

    inline bool in_float_layout() const { return layout == LT_FLOAT; }

    void arrange() const override;

    inline user_workspace_t& add_client(client_ptr_t client) override
    {
        clients.add(client);
        ::std::for_each(client->children.begin(), client->children.end(),
            [=](client_ptr_t child) { clients.add(child); });

        return *this;
    }

    inline user_workspace_t& remove_client(client_ptr_t client) override
    {
        ::std::for_each(client->children.begin(), client->children.end(),
            [=](client_ptr_t child) { clients.remove(child); });
        clients.remove(client);

        return *this;
    }

    user_workspace_t& set_n_master(unsigned);
    user_workspace_t& set_gap_size(unsigned);
    user_workspace_t& set_m_factor(float);
    user_workspace_t& set_m1_weight(unsigned);
    user_workspace_t& set_layout(layouttype);

    void map_clients();
    void unmap_clients();

    void activate();
    void deactivate();

    inline bool empty() const { return clients.get_all().empty(); }
    inline const client_ptr_t get_focused() const { return clients.get(); }
    inline void set(client_ptr_t client) { clients.set(client); }
    inline void unset() { clients.unset(); }
    inline bool contains(client_ptr_t client) const { return clients.contains(client); }

private:
    unsigned      number;
    ::std::string name;
    unsigned      n_master;
    unsigned      gap_size;
    unsigned      m1_weight;
    float         m_factor;
    bool          mirrored;
    layouttype    layout;
    layouttype    previous_layout;
    focus_cycle   clients;

}* user_workspace_ptr_t;


inline user_workspace_ptr_t user_workspace(workspace_ptr_t workspace)
{
    return dynamic_cast<user_workspace_ptr_t>(workspace);
}


inline bool is_user_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == USER_WORKSPACE;
}

#endif//__KRANEWM_WORKSPACE_GUARD__
