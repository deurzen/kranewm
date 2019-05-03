#ifndef __KRANEWM_WORKSPACE_GUARD__
#define __KRANEWM_WORKSPACE_GUARD__

#include "client.hh"
#include "layout.hh"
#include "focus-cycle.hh"

#include <map>
#include <string>


static const ::std::map<unsigned, ::std::string> USER_WORKSPACES({
//    nr   name
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


enum class workspacetype_t
{
    generic,
    move,
    resize,
    user,
};

typedef class workspace_t
{
public:
    explicit workspace_t(workspacetype_t _type = workspacetype_t::generic)
        : type(_type) {}

    virtual ~workspace_t() {}

    virtual workspacetype_t get_type() const { return type; }

    virtual void arrange() const {}

    virtual workspace_t& add_client(client_ptr_t)    { return *this; }
    virtual workspace_t& remove_client(client_ptr_t) { return *this; }

private:
    workspacetype_t type;

}* workspace_ptr_t;

typedef class moveresize_workspace_t : public workspace_t
{
public:
    explicit moveresize_workspace_t(workspacetype_t _type)
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


inline moveresize_workspace_ptr_t moveresize_workspace(workspace_ptr_t workspace)
{
    return dynamic_cast<moveresize_workspace_ptr_t>(workspace);
}

inline bool is_move_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::move;
}

inline bool is_resize_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::resize;
}

inline bool is_moveresize_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::move
        || workspace->get_type() == workspacetype_t::resize;
}


typedef class user_workspace_t : public workspace_t
{
public:
    user_workspace_t(unsigned _number, ::std::string&& _name, ewmh_t& ewmh)
        : workspace_t(workspacetype_t::user), number(_number), name(_name),
          n_master(1), gap_size(0), m1_weight(1), m_factor(.5f), mirrored(false),
          layout(layout_t::floating), previous_layout(layout),
          layouthandler(layouthandler_t{ewmh})
    {}

    void arrange() const override;

    unsigned get_number() const;
    const ::std::deque<client_ptr_t>& get_all() const;

    bool empty() const;
    bool contains(client_ptr_t client) const;
    bool is_mirrored() const;

    bool in_float_layout() const;
    bool in_monocle_layout() const;

    const client_ptr_t get_focused() const;
    void set_focused(client_ptr_t client, bool = false);
    void unset_focused();

    user_workspace_t& add_client(client_ptr_t) override;
    user_workspace_t& remove_client(client_ptr_t) override;

    user_workspace_t& forward();
    user_workspace_t& backward();
    user_workspace_t& move_forward();
    user_workspace_t& move_backward();

    user_workspace_t& rotate_stack_forward();
    user_workspace_t& rotate_stack_backward();
    user_workspace_t& rotate_master_forward();
    user_workspace_t& rotate_master_backward();

    user_workspace_t& zoom();
    user_workspace_t& mirror();
    user_workspace_t& jump_pane();

    user_workspace_t& set_n_master(unsigned);
    user_workspace_t& set_gap_size(unsigned);
    user_workspace_t& set_m_factor(float);
    user_workspace_t& set_m1_weight(unsigned);
    user_workspace_t& set_layout(layout_t);

    unsigned get_n_master() const;
    unsigned get_gap_size() const;
    float get_m_factor() const;
    unsigned get_m1_weight() const;
    layout_t get_layout() const;

    bool is_master(client_ptr_t);
    bool is_stack(client_ptr_t);

    bool master_focused();
    bool stack_focused();

private:
    unsigned        number;
    ::std::string   name;
    unsigned        n_master;
    unsigned        gap_size;
    unsigned        m1_weight;
    float           m_factor;
    bool            mirrored;
    focus_cycle_t   clients;
    layout_t        layout;
    layout_t        previous_layout;
    layouthandler_t layouthandler;

}* user_workspace_ptr_t;


inline user_workspace_ptr_t user_workspace(workspace_ptr_t workspace)
{
    return dynamic_cast<user_workspace_ptr_t>(workspace);
}


inline bool is_user_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::user;
}

#endif//__KRANEWM_WORKSPACE_GUARD__
