#ifndef __STACK_H_GUARD__
#define __STACK_H_GUARD__

#include "../winsys/window.hh"

#include <unordered_map>
#include <vector>

class StackHandler final
{
public:
    enum class StackLayer
    {
        Desktop,
        Below_,
        Dock,
        // Regular,
        // Free,
        // Transient,
        Above_,
        // Fullscreen,
        Notification,
    };

    StackHandler();
    ~StackHandler();

    void add_window(winsys::Window, StackLayer);
    void remove_window(winsys::Window);

    void relayer_window(winsys::Window, StackLayer);
    void raise_window(winsys::Window);

    void add_above_other(winsys::Window, winsys::Window);
    void add_below_other(winsys::Window, winsys::Window);

    std::vector<winsys::Window> const& get_layer(StackLayer) const;

private:
    std::unordered_map<winsys::Window, StackLayer> m_layers;

    std::vector<winsys::Window> m_desktop;
    std::vector<winsys::Window> m_below;
    std::vector<winsys::Window> m_dock;
    std::vector<winsys::Window> m_above;
    std::vector<winsys::Window> m_notification;

    std::unordered_map<winsys::Window, winsys::Window> m_above_other;
    std::unordered_map<winsys::Window, winsys::Window> m_below_other;

};

#endif//__STACK_H_GUARD__
