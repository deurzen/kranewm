#ifndef __KRANEWM_WORKSPACE_GUARD__
#define __KRANEWM_WORKSPACE_GUARD__

#include "client.hh"
#include "focus-cycle.hh"
#include "layout.hh"
#include "stack.hh"

#include <map>
#include <string>

#define NOWORKSPACE nullptr

static const ::std::map<::std::size_t, ::std::string> USER_WORKSPACES({
//    nr   name
    { 1,   "main" },
    { 2,   "web"  },
    { 3,   "term" },
    { 4,   ""     },
    { 5,   ""     },
    { 6,   ""     },
    { 7,   ""     },
    { 8,   ""     },
    { 9,   ""     },
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
      : type(_type)
    {}

    virtual ~workspace_t()
    {}

    virtual workspacetype_t
    get_type() const
    {
        return type;
    }

    virtual void
    arrange() const
    {}

    virtual workspace_t&
    add_client(client_ptr_t)
    {
        return *this;
    }

    virtual workspace_t&
    remove_client(client_ptr_t)
    {
        return *this;
    }

    virtual workspace_t&
    add_family(client_ptr_t)
    {
        return *this;
    }

    virtual workspace_t&
    remove_family(client_ptr_t)
    {
        return *this;
    }

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

    void arrange() const override
    {}

    inline void
    set(client_ptr_t _client)
    {
        client = _client;
    }

    inline void
    unset()
    {
        client = nullptr;
    }

    inline client_ptr_t
    get() const
    {
        return client;
    }

    inline bool
    is_set() const
    {
        return client != nullptr;
    }

    inline moveresize_workspace_t&
    add_client(client_ptr_t client) override
    {
        if (!is_set())
            set(client);

        return *this;
    }

    inline moveresize_workspace_t&
    remove_client(client_ptr_t client) override
    {
        if (is_set() && client == get()) {
            unset();
        }

        return *this;
    }

    inline moveresize_workspace_t&
    add_family(client_ptr_t client) override
    {
        add_client(client);
        return *this;
    }

    inline moveresize_workspace_t&
    remove_family(client_ptr_t client) override
    {
        remove_client(client);
        return *this;
    }

private:
    client_ptr_t client;

}* moveresize_workspace_ptr_t;


inline moveresize_workspace_ptr_t
moveresize_workspace(workspace_ptr_t workspace)
{
    return dynamic_cast<moveresize_workspace_ptr_t>(workspace);
}

inline bool
is_move_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::move;
}

inline bool
is_resize_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::resize;
}

inline bool
is_moveresize_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::move
        || workspace->get_type() == workspacetype_t::resize;
}


typedef class user_workspace_t : public workspace_t
{
    const ::std::size_t DEFAULT_NMASTER = 1;
    const ::std::size_t DEFAULT_GAPSIZE = 0;
    const float DEFAULT_MFACTOR = .5f;
    const bool DEFAULT_MIRRORED = false;
    const bool DEFAULT_SIDEBARSET = true;
    const layout_t DEFAULT_LAYOUT = layout_t::floating;

public:
    user_workspace_t(::std::size_t _number, ::std::string&& _name, ewmh_t& ewmh)
      : workspace_t(workspacetype_t::user),
        m_number(_number),
        m_name(_name),
        m_nurgent(0),
        m_nmaster(DEFAULT_NMASTER),
        m_gapsize(DEFAULT_GAPSIZE),
        m_mfactor(DEFAULT_MFACTOR),
        m_mirrored(DEFAULT_MIRRORED),
        m_sidebarset(DEFAULT_SIDEBARSET),
        m_layout(DEFAULT_LAYOUT),
        m_previous_layout(m_layout),
        m_layouthandler(layouthandler_t{ewmh})
    {}

    void arrange() const override;

    ::std::size_t get_number() const;
    ::std::size_t get_index() const;

    const ::std::deque<client_ptr_t>& get_all() const;
    const ::std::deque<client_ptr_t>& get_icons() const;
    const ::std::deque<client_ptr_t>& get_disowned() const;

    bool empty() const;
    bool contains(client_ptr_t) const;
    bool is_mirrored() const;
    bool has_sidebar() const;

    bool in_float_layout() const;
    bool in_monocle_layout() const;

    const client_ptr_t get_focused() const;
    void set_focused(client_ptr_t, bool = false);
    void unset_focused();

    user_workspace_t& add_client(client_ptr_t) override;
    user_workspace_t& remove_client(client_ptr_t) override;
    user_workspace_t& add_family(client_ptr_t) override;
    user_workspace_t& remove_family(client_ptr_t) override;

    user_workspace_t& add_icon(client_ptr_t);
    user_workspace_t& remove_icon(client_ptr_t);
    user_workspace_t& add_disowned(client_ptr_t);
    user_workspace_t& remove_disowned(client_ptr_t);
    user_workspace_t& raise_client(client_ptr_t);

    user_workspace_t& forward();
    user_workspace_t& backward();
    user_workspace_t& move_forward();
    user_workspace_t& move_backward();

    user_workspace_t& rotate_clients_forward();
    user_workspace_t& rotate_clients_backward();
    user_workspace_t& rotate_stack_forward();
    user_workspace_t& rotate_stack_backward();
    user_workspace_t& rotate_master_forward();
    user_workspace_t& rotate_master_backward();

    user_workspace_t& zoom();
    user_workspace_t& mirror();
    user_workspace_t& sidebarset(bool);
    user_workspace_t& jump_pane();

    user_workspace_t& set_nmaster(::std::size_t);
    user_workspace_t& set_gapsize(::std::size_t);
    user_workspace_t& set_mfactor(float);
    user_workspace_t& set_layout(layout_t);

    ::std::size_t get_nmaster() const;
    ::std::size_t get_gapsize() const;
    float get_mfactor() const;
    layout_t get_layout() const;
    const workspacestack_t& get_stack() const;

    ::std::size_t get_nurgent() const;
    bool is_urgent() const;
    void record_urgent();
    void erase_urgent();

    bool is_master(client_ptr_t);
    bool is_stack(client_ptr_t);

    bool master_focused();
    bool stack_focused();

    bool reset();

private:
    ::std::size_t    m_number;
    ::std::string    m_name;
    ::std::size_t    m_nurgent;
    ::std::size_t    m_nmaster;
    ::std::size_t    m_gapsize;
    float            m_mfactor;
    bool             m_mirrored;
    bool             m_sidebarset;
    focus_cycle_t    m_clients;
    focus_cycle_t    m_icons;
    focus_cycle_t    m_disowned;
    workspacestack_t m_stack;
    layout_t         m_layout;
    layout_t         m_previous_layout;
    layouthandler_t  m_layouthandler;

}* user_workspace_ptr_t;


inline user_workspace_ptr_t
user_workspace(workspace_ptr_t workspace)
{
    return dynamic_cast<user_workspace_ptr_t>(workspace);
}


inline bool
is_user_workspace(workspace_ptr_t workspace)
{
    return workspace->get_type() == workspacetype_t::user;
}

#endif//__KRANEWM_WORKSPACE_GUARD__
