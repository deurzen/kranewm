#include "stack.hh"
#include "util.hh"


windowstack_t&
windowstack_t::add_to_stack(windowstack_window_t win)
{
    if (m_win_layers.count(win.win))
        return *this;

    switch (win.layer) {
    case layer_t::desktop:      m_desktop_windows.push_back(win.win);      break;
    case layer_t::below:        m_below_windows.push_back(win.win);        break;
    case layer_t::normal:       m_normal_windows.push_back(win.win);       break;
    case layer_t::floating:     m_floating_windows.push_back(win.win);     break;
    case layer_t::dock:         m_dock_windows.push_back(win.win);         break;
    case layer_t::indicator:    m_indicator_windows.push_back(win.win);    break;
    case layer_t::above:        m_above_windows.push_back(win.win);        break;
    case layer_t::fullscreen:   m_fullscreen_windows.push_back(win.win);   break;
    case layer_t::notification: m_notification_windows.push_back(win.win); break;
    }

    m_win_layers[win.win] = win.layer;
    return *this;
}

windowstack_t&
windowstack_t::remove_from_stack(x_wrapper::window_t win)
{
    if (!m_win_layers.count(win))
        return *this;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      erase_find(m_desktop_windows, win);      break;
    case layer_t::below:        erase_find(m_below_windows, win);        break;
    case layer_t::normal:       erase_find(m_normal_windows, win);       break;
    case layer_t::floating:     erase_find(m_floating_windows, win);     break;
    case layer_t::dock:         erase_find(m_dock_windows, win);         break;
    case layer_t::indicator:    erase_find(m_indicator_windows, win);    break;
    case layer_t::above:        erase_find(m_above_windows, win);        break;
    case layer_t::fullscreen:   erase_find(m_fullscreen_windows, win);   break;
    case layer_t::notification: erase_find(m_notification_windows, win); break;
    }

    erase_find(m_win_layers, win);

    return *this;
}

windowstack_t&
windowstack_t::relayer_window(windowstack_window_t win)
{
    remove_from_stack(win.win);
    add_to_stack(win);
    return *this;
}

windowstack_t&
windowstack_t::raise_window(x_wrapper::window_t win)
{
    if (!m_win_layers.count(win))
        return *this;

    switch (m_win_layers[win]) {
    case layer_t::desktop:
        m_desktop_windows.remove(win);
        m_desktop_windows.push_front(win);
        break;
    case layer_t::below:
        m_below_windows.remove(win);
        m_below_windows.push_front(win);
        break;
    case layer_t::normal:
        m_normal_windows.remove(win);
        m_normal_windows.push_front(win);
        break;
    case layer_t::floating:
        m_floating_windows.remove(win);
        m_floating_windows.push_front(win);
        break;
    case layer_t::dock:
        m_dock_windows.remove(win);
        m_dock_windows.push_front(win);
        break;
    case layer_t::indicator:
        m_indicator_windows.remove(win);
        m_indicator_windows.push_front(win);
        break;
    case layer_t::above:
        m_above_windows.remove(win);
        m_above_windows.push_front(win);
        break;
    case layer_t::fullscreen:
        m_fullscreen_windows.remove(win);
        m_fullscreen_windows.push_front(win);
        break;
    case layer_t::notification:
        m_notification_windows.remove(win);
        m_notification_windows.push_front(win);
        break;
    }

    return *this;
}

windowstack_t&
windowstack_t::lower_window(x_wrapper::window_t win)
{
    if (!m_win_layers.count(win))
        return *this;

    switch (m_win_layers[win]) {
    case layer_t::desktop:      splice_back(m_desktop_windows, win);      break;
    case layer_t::below:        splice_back(m_below_windows, win);        break;
    case layer_t::normal:       splice_back(m_normal_windows, win);       break;
    case layer_t::floating:     splice_back(m_floating_windows, win);     break;
    case layer_t::dock:         splice_back(m_dock_windows, win);         break;
    case layer_t::indicator:    splice_back(m_indicator_windows, win);    break;
    case layer_t::above:        splice_back(m_above_windows, win);        break;
    case layer_t::fullscreen:   splice_back(m_fullscreen_windows, win);   break;
    case layer_t::notification: splice_back(m_notification_windows, win); break;
    }

    return *this;
}

::std::list<x_wrapper::window_t>
windowstack_t::get_all_of_type(layer_t type)
{
    switch (type) {
    case layer_t::desktop:      return m_desktop_windows;
    case layer_t::below:        return m_below_windows;
    case layer_t::normal:       return m_normal_windows;
    case layer_t::floating:     return m_floating_windows;
    case layer_t::dock:         return m_dock_windows;
    case layer_t::indicator:    return m_indicator_windows;
    case layer_t::above:        return m_above_windows;
    case layer_t::fullscreen:   return m_fullscreen_windows;
    case layer_t::notification: return m_notification_windows;
    default: return m_normal_windows;
    }
}

void
windowstack_t::apply()
{
    size_t n = m_win_layers.size();
    ::std::vector<Window> wins;
    wins.reserve(n);

    wins.insert(wins.end(), m_notification_windows.begin(), m_notification_windows.end());
    wins.insert(wins.end(), m_fullscreen_windows.begin(),   m_fullscreen_windows.end());
    wins.insert(wins.end(), m_above_windows.begin(),        m_above_windows.end());
    wins.insert(wins.end(), m_floating_windows.begin(),     m_floating_windows.end());
    wins.insert(wins.end(), m_normal_windows.begin(),       m_normal_windows.end());
    wins.insert(wins.end(), m_indicator_windows.begin(),    m_indicator_windows.end());
    wins.insert(wins.end(), m_dock_windows.begin(),         m_dock_windows.end());
    wins.insert(wins.end(), m_below_windows.begin(),        m_below_windows.end());
    wins.insert(wins.end(), m_desktop_windows.begin(),      m_desktop_windows.end());

    x_wrapper::restack_windows(wins.data(), n);
}
