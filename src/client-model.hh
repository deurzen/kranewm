#ifndef __KRANEWM_CLIENT_MODEL_GUARD__
#define __KRANEWM_CLIENT_MODEL_GUARD__

#include "changes.hh"
#include "x-wrapper/window.hh"
#include "workspace.hh"
#include "rule.hh"

#include <unordered_map>
#include <vector>


class client_model_t
{
public:
    explicit client_model_t(changequeue_t& changequeue)
        : m_changequeue(changequeue),
          m_current_workspace(nullptr),
          m_move_workspace(new moveresize_workspace_t{workspacetype_t::move}),
          m_resize_workspace(new moveresize_workspace_t{workspacetype_t::resize}),
          m_marked_client(nullptr),
          m_focused_client(nullptr)
        {
            for (auto&& [nr,name] : USER_WORKSPACES)
                m_user_workspaces.push_back(new user_workspace_t{nr, name.c_str()});

            m_current_workspace = m_user_workspaces.front();
        }

    client_ptr_t win_to_client(x_wrapper::window_t);
    workspace_ptr_t client_workspace(client_ptr_t);
    user_workspace_ptr_t client_user_workspace(client_ptr_t);

    user_workspace_ptr_t active_workspace() const;
    client_ptr_t focused_client() const;

    void manage_client(client_ptr_t, rule_t);
    void unmanage_client(client_ptr_t);

    void focus(client_ptr_t);
    void unfocus();
    void unfocus_if_focused(client_ptr_t);

    void cycle_focus_forward();
    void cycle_focus_backward();

    void start_moving(client_ptr_t);
    void start_resizing(client_ptr_t);
    void stop_moving(client_ptr_t, pos_t = pos_t{});
    void stop_resizing(client_ptr_t, pos_t = pos_t{}, dim_t = dim_t{});

    void client_to_workspace(client_ptr_t, unsigned);
    void client_to_workspace(client_ptr_t, workspace_ptr_t);
    void change_active_workspace(unsigned);
    void change_active_workspace(user_workspace_ptr_t);

private:
    void sync_workspace_focus();

    changequeue_t& m_changequeue;

    user_workspace_ptr_t m_current_workspace;

    moveresize_workspace_ptr_t m_move_workspace;
    moveresize_workspace_ptr_t m_resize_workspace;

    ::std::vector<x_wrapper::window_t> m_managed_windows;
    ::std::vector<user_workspace_ptr_t> m_user_workspaces;

    ::std::unordered_map<x_wrapper::window_t, client_ptr_t> m_client_windows;
    ::std::unordered_map<client_ptr_t, user_workspace_ptr_t> m_client_workspaces;

    client_ptr_t m_marked_client;
    client_ptr_t m_focused_client;

};

#endif//__KRANEWM_CLIENT_MODEL_GUARD__
