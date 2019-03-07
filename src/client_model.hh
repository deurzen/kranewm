#ifndef __KRANEWM_CLIENT_MODEL_GUARD__
#define __KRANEWM_CLIENT_MODEL_GUARD__

#include "x_wrapper/window.hh"
#include "workspace.hh"

#include <map>
#include <vector>

// TODO administration of clients/workspaces : general overview, combines everything higher-level


class client_model
{
public:
    client_model()
        : m_current_workspace(nullptr),
          m_move_workspace(new moveresize_workspace_t{MOVE_WORKSPACE}),
          m_resize_workspace(new moveresize_workspace_t{RESIZE_WORKSPACE}),
          m_marked_client(nullptr),
          m_focused_client(nullptr)
        {
            for (auto&& [nr,name] : USER_WORKSPACES)
                m_user_workspaces.push_back(new user_workspace_t{nr, name.c_str()});
        }

    client_ptr_t win_to_client(x_wrapper::window_t);
    workspace_ptr_t client_workspace(client_ptr_t);

    void focus(client_ptr_t);

    void start_moving(client_ptr_t);
    void stop_moving(client_ptr_t, Pos);
    void start_resizing(client_ptr_t);
    void stop_resizing(client_ptr_t, Pos, Size);

private:
    user_workspace_ptr_t m_current_workspace;

    moveresize_workspace_ptr_t m_move_workspace;
    moveresize_workspace_ptr_t m_resize_workspace;

    ::std::vector<x_wrapper::window_t> m_registered_windows;
    ::std::vector<user_workspace_ptr_t> m_user_workspaces;

    ::std::map<x_wrapper::window_t, client_ptr_t> m_client_windows;
    ::std::map<client_ptr_t, workspace_ptr_t> m_client_workspaces;

    client_ptr_t m_marked_client;
    client_ptr_t m_focused_client;

};

#endif//__KRANEWM_CLIENT_MODEL_GUARD__
