#ifndef __KRANEWM_CLIENT_MODEL_GUARD__
#define __KRANEWM_CLIENT_MODEL_GUARD__

#include "stack.hh"
#include "context.hh"
#include "workspace.hh"
#include "profile.hh"

#include "x-data/window.hh"

#include <unordered_map>
#include <vector>

// fwd decls
class changequeue_t;
class windowstack_t;
class processjumplist_t;
class rule_t;


class client_model_t
{
public:
    explicit client_model_t(changequeue_t& changequeue, ewmh_t& ewmh,
        windowstack_t& windowstack, processjumplist_t& processes)
        : m_changequeue(changequeue),
          m_windowstack(windowstack),
          m_processes(processes),
          m_current_context(new context_t{'a', CONTEXTS.at('a'), ewmh, true}),
          m_current_workspace(m_current_context->get_activated()),
          m_move_workspace(new moveresize_workspace_t{workspacetype_t::move}),
          m_resize_workspace(new moveresize_workspace_t{workspacetype_t::resize}),
          m_user_workspaces(m_current_context->get_workspaces()),
          m_profiles(3, profile_t{}),
          m_focused_client(nullptr)
    {
        m_contexts.push_back(m_current_context);
        for (::std::size_t i = 1; i < CONTEXTS.size(); ++i) {
            char lt = 'a' + static_cast<char>(i);
            m_contexts.push_back(new context_t{lt, CONTEXTS.at(lt), ewmh});
        }
    }

    ~client_model_t()
    {
        delete m_move_workspace;
        delete m_resize_workspace;

        for (auto& context : m_contexts)
            delete context;
    }

    client_ptr_t win_client(x_data::window_t);
    context_ptr_t client_context(client_ptr_t);
    workspace_ptr_t client_workspace(client_ptr_t);
    user_workspace_ptr_t client_user_workspace(client_ptr_t);

    user_workspace_ptr_t active_workspace() const;
    context_ptr_t active_context() const;
    client_ptr_t focused_client() const;

    void manage_client(client_ptr_t, rule_t);
    void unmanage_client(client_ptr_t);

    void focus(client_ptr_t, bool = false);
    void unfocus();
    void unfocus_if_focused(client_ptr_t);

    void cycle_focus_forward();
    void cycle_focus_backward();

    void start_moving(client_ptr_t);
    void start_resizing(client_ptr_t);
    void stop_moving(client_ptr_t, pos_t = pos_t{});
    void stop_resizing(client_ptr_t, pos_t = pos_t{}, dim_t = dim_t{});

    void client_to_workspace(client_ptr_t, ::std::size_t);
    void client_to_workspace(client_ptr_t, workspace_ptr_t);
    void change_active_workspace(::std::size_t, bool = true);
    void change_active_workspace(user_workspace_ptr_t = nullptr, bool = true);

    void client_to_context(client_ptr_t, ::std::size_t);
    void client_to_context(client_ptr_t, context_ptr_t);
    void change_active_context(::std::size_t);
    void change_active_context(context_ptr_t = nullptr);

    void set_fullscreen(client_ptr_t, clientaction_t);
    void set_urgent(client_ptr_t, clientaction_t);
    void set_iconified(client_ptr_t, clientaction_t, bool = true);
    void set_disowned(client_ptr_t, clientaction_t, bool = true);
    void set_sticky(client_ptr_t, clientaction_t, bool = true);
    void set_marked(client_ptr_t);

    void wedge_clients();
    void refullscreen_clients();

    void save_profile(::std::size_t);
    void apply_profile(::std::size_t);

    void jump_marked();
    void jump_process(const ::std::string&);

    void sync_workspace_focus(bool = false);

private:
    changequeue_t& m_changequeue;
    windowstack_t& m_windowstack;
    processjumplist_t& m_processes;

    context_ptr_t m_current_context;
    user_workspace_ptr_t m_current_workspace;

    moveresize_workspace_ptr_t m_move_workspace;
    moveresize_workspace_ptr_t m_resize_workspace;

    ::std::vector<x_data::window_t> m_managed_windows;
    ::std::vector<context_ptr_t> m_contexts;
    const ::std::vector<user_workspace_ptr_t>* m_user_workspaces;

    ::std::unordered_map<x_data::window_t, client_ptr_t> m_client_windows;
    ::std::unordered_map<client_ptr_t, context_ptr_t> m_client_contexts;
    ::std::unordered_map<client_ptr_t, user_workspace_ptr_t> m_client_workspaces;

    ::std::unordered_map<client_ptr_t, client_t> m_fullscreen_clients;
    ::std::vector<client_ptr_t> m_sticky_clients;

    ::std::vector<profile_t> m_profiles;

    client_ptr_t m_focused_client;

};

#endif//__KRANEWM_CLIENT_MODEL_GUARD__
