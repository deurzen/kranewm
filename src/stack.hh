#ifndef __KRANEWM_STACK_GUARD__
#define __KRANEWM_STACK_GUARD__

#include "x-wrapper/window.hh"
#include <list>
#include <unordered_map>


enum class layer_t
{ // bottom to top
    desktop,
    below,
    normal,
    floating,
    dock,
    indicator,
    above,
    fullscreen,
    notification
};

// Forward decl
struct client_t;
typedef client_t* client_ptr_t;


struct windowstack_window_t
{
    windowstack_window_t(x_wrapper::window_t _win, layer_t _layer)
        : win(_win),
          layer(_layer) {}

    x_wrapper::window_t win;
    layer_t layer;

};


class windowstack_t
{
public:
    windowstack_t& add_to_stack(windowstack_window_t);
    windowstack_t& remove_from_stack(x_wrapper::window_t);
    windowstack_t& relayer_window(windowstack_window_t);
    windowstack_t& raise_window(x_wrapper::window_t);
    windowstack_t& lower_window(x_wrapper::window_t);

    ::std::list<x_wrapper::window_t> get_all_of_type(layer_t);
    void apply();

private:
    ::std::unordered_map<x_wrapper::window_t, layer_t> m_win_layers;

    ::std::list<x_wrapper::window_t> m_desktop_windows;
    ::std::list<x_wrapper::window_t> m_below_windows;
    ::std::list<x_wrapper::window_t> m_normal_windows;
    ::std::list<x_wrapper::window_t> m_floating_windows;
    ::std::list<x_wrapper::window_t> m_dock_windows;
    ::std::list<x_wrapper::window_t> m_indicator_windows;
    ::std::list<x_wrapper::window_t> m_above_windows;
    ::std::list<x_wrapper::window_t> m_fullscreen_windows;
    ::std::list<x_wrapper::window_t> m_notification_windows;

};


#endif//__KRANEWM_STACK_GUARD__
