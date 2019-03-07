#ifndef __KRANEWM_WORKSPACE_GUARD__
#define __KRANEWM_WORKSPACE_GUARD__

#include "client.hh"

// TODO workspace->arrange

enum WorkspaceType
{
    WORKSPACE = 0,
    MOVE_WORKSPACE,
    RESIZE_WORKSPACE,
    ICON_WORKSPACE,
    USER_WORKSPACE,
    SCRATCHPAD
};


typedef class workspace_t
{
public:
    explicit workspace_t(WorkspaceType _type = WORKSPACE)
        : type(_type) {}

    virtual ~workspace_t() {}

    virtual WorkspaceType get_type() const { return type; }

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

    inline void set(client_ptr_t _client) { client = _client; }
    inline void unset() { client = nullptr; }

    inline bool is_set() const { return client != nullptr; }

private:
    client_ptr_t client;

}* moveresize_workspace_ptr_t;


typedef struct user_workspace_t : public workspace_t
{
    user_workspace_t(unsigned _number, ::std::string&& _name)
      : workspace_t(USER_WORKSPACE),
        number(_number), name(_name)
    {}

    /* inline void */
    /* set_layout(Layout _layout) */
    /* { */
    /*     Layout current_layout = layout; */
    /*     if (_layout == LT_TOGGLE) */
    /*         layout = previous_layout; */
    /*     else */
    /*         layout = _layout; */

    /*     previous_layout = current_layout; */
    /* } */

    unsigned number;
    ::std::string name;
    bool mirrored = false;
    unsigned n_master = 1;
    int gap_size = 5;
    float m_ratio = 0.6f;
    float m_factor = 0;
    /* Layout layout = LT_FLOAT; */
    /* Layout previous_layout = LT_FLOAT; */
    /* FocusGroup clients; */
    /* Iconworkspace_t icons; */
}* user_workspace_ptr_t;


#endif//__KRANEWM_WORKSPACE_GUARD__
