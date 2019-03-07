#ifndef __KRANEWM_WORKSPACE_GUARD__
#define __KRANEWM_WORKSPACE_GUARD__

#include "client.hh"
#include "focus_cycle.hh"

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


enum WorkspaceType
{
    WORKSPACE = 0,
    MOVE_WORKSPACE,
    RESIZE_WORKSPACE,
    ICON_WORKSPACE,
    USER_WORKSPACE,
    SCRATCHPAD
};

enum LayoutType {
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
    explicit workspace_t(WorkspaceType _type = WORKSPACE)
        : type(_type) {}

    virtual ~workspace_t() {}

    virtual WorkspaceType get_type() const { return type; }

    virtual void arrange() const {}

private:
    WorkspaceType type;

}* workspace_ptr_t;

typedef class moveresize_workspace_t : public workspace_t
{
public:
    explicit moveresize_workspace_t(WorkspaceType _type)
        : workspace_t(_type),
          client(nullptr)
    {}

    void arrange() const override {}

    inline void set(client_ptr_t _client) { client = _client; }
    inline void unset() { client = nullptr; }

    inline bool is_set() const { return client != nullptr; }

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


typedef class user_workspace_t : public workspace_t
{
public:
    user_workspace_t(unsigned _number, ::std::string&& _name)
        : workspace_t(USER_WORKSPACE), number(_number), name(_name),
          n_master(1), gap_size(5), m1_weight(1), m_factor(.6f),
          mirrored(false), layout(LT_FLOAT), previous_layout(layout)
    {}

    void arrange() const override;

    user_workspace_t& register_client(client_ptr_t);
    user_workspace_t& unregister_client(client_ptr_t);

    user_workspace_t& set_n_master(unsigned);
    user_workspace_t& set_gap_size(unsigned);
    user_workspace_t& set_m_factor(float);
    user_workspace_t& set_m1_weight(unsigned);
    user_workspace_t& set_layout(LayoutType);

    void map_clients();
    void unmap_clients();

    void activate();
    void deactivate();

private:
    unsigned      number;
    ::std::string name;
    unsigned      n_master;
    unsigned      gap_size;
    unsigned      m1_weight;
    float         m_factor;
    bool          mirrored;
    LayoutType    layout;
    LayoutType    previous_layout;
    focus_cycle   clients;

}* user_workspace_ptr_t;


inline bool is_user_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == USER_WORKSPACE;
}

#endif//__KRANEWM_WORKSPACE_GUARD__
