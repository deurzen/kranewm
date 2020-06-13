#ifndef __KRANEWM_STACK_GUARD__
#define __KRANEWM_STACK_GUARD__

#include "x-data/window.hh"

#include <list>
#include <unordered_map>


enum class layer_t
{ // bottom to top
    desktop,
    below,
    dock,
    above,
    notification
};

// fwd decls
typedef struct client_t* client_ptr_t;
typedef struct user_workspace_t* user_workspace_ptr_t;


struct windowstack_window_t final
{
    windowstack_window_t(x_data::window_t _win, layer_t _layer)
      : win(_win),
        layer(_layer)
    {}

    x_data::window_t win;
    layer_t layer;

};

class workspacestack_t final
{
public:
    workspacestack_t() = default;

    void add(client_ptr_t);
    void remove(client_ptr_t);
    void raise(client_ptr_t);

    ::std::vector<client_ptr_t>& get_clients();

private:
    ::std::vector<client_ptr_t> m_clients;

};


class windowstack_t final
{
public:
    windowstack_t& add_to_stack(windowstack_window_t);
    windowstack_t& remove_from_stack(x_data::window_t);
    windowstack_t& relayer_window(windowstack_window_t);

    windowstack_t& raise_window(x_data::window_t);
    windowstack_t& lower_window(x_data::window_t);

    windowstack_t& raise_window_above(x_data::window_t, x_data::window_t);
    windowstack_t& lower_window_below(x_data::window_t, x_data::window_t);

    bool occluded_by(x_data::window_t, x_data::window_t);
    bool occluded_by_any(x_data::window_t);
    bool occludes_any(x_data::window_t);

    ::std::list<x_data::window_t> get_all_of_type(layer_t);

    void apply(user_workspace_ptr_t);
    void apply(workspacestack_t, bool = false);

private:
    ::std::unordered_map<x_data::window_t, layer_t> m_win_layers;

    ::std::list<x_data::window_t> m_desktop_windows;
    ::std::list<x_data::window_t> m_below_windows;
    ::std::list<x_data::window_t> m_dock_windows;
    ::std::list<x_data::window_t> m_above_windows;
    ::std::list<x_data::window_t> m_notification_windows;

};


#endif//__KRANEWM_STACK_GUARD__
