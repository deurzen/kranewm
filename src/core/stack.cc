#include "stack.hh"
#include "../winsys/util.hh"

StackHandler::StackHandler()
    : m_layers({}),
      m_desktop({}),
      m_below({}),
      m_dock({}),
      m_above({}),
      m_notification({}),
      m_above_other({}),
      m_below_other({})
{}

StackHandler::~StackHandler()
{}

void
StackHandler::add_window(winsys::Window window, StackLayer layer)
{
    if (!(m_layers.count(window) > 0)) {
        std::vector<winsys::Window>* windows;

        switch (layer) {
        case StackLayer::Desktop:      windows = &m_desktop;      break;
        case StackLayer::Below_:       windows = &m_below;        break;
        case StackLayer::Dock:         windows = &m_dock;         break;
        case StackLayer::Above_:       windows = &m_above;        break;
        case StackLayer::Notification: windows = &m_notification; break;
        default: Util::die("no associated layer");
        }

        windows->push_back(window);
        m_layers[window] = layer;
    }
}

void
StackHandler::remove_window(winsys::Window window)
{
    if (m_layers.count(window) > 0) {
        StackLayer layer = m_layers.at(window);
        std::vector<winsys::Window>* windows;

        switch (layer) {
        case StackLayer::Desktop:      windows = &m_desktop;      break;
        case StackLayer::Below_:       windows = &m_below;        break;
        case StackLayer::Dock:         windows = &m_dock;         break;
        case StackLayer::Above_:       windows = &m_above;        break;
        case StackLayer::Notification: windows = &m_notification; break;
        default: Util::die("no associated layer");
        }

        Util::erase_remove(*windows, window);
        m_layers.erase(window);
    }

    m_above_other.erase(window);
    m_below_other.erase(window);
}


void
StackHandler::relayer_window(winsys::Window window, StackLayer layer)
{
    remove_window(window);
    add_window(window, layer);
}

void
StackHandler::raise_window(winsys::Window window)
{
    if (m_layers.count(window) > 0) {
        StackLayer layer = m_layers.at(window);
        std::vector<winsys::Window>* windows;

        switch (layer) {
        case StackLayer::Desktop:      windows = &m_desktop;      break;
        case StackLayer::Below_:       windows = &m_below;        break;
        case StackLayer::Dock:         windows = &m_dock;         break;
        case StackLayer::Above_:       windows = &m_above;        break;
        case StackLayer::Notification: windows = &m_notification; break;
        default: Util::die("no associated layer");
        }

        Util::erase_remove(*windows, window);
        windows->push_back(window);
    }
}


void
StackHandler::add_above_other(winsys::Window window, winsys::Window sibling)
{
    if (!(m_above_other.count(window) > 0))
        m_above_other[window] = sibling;
}

void
StackHandler::add_below_other(winsys::Window window, winsys::Window sibling)
{
    if (!(m_below_other.count(window) > 0))
        m_below_other[window] = sibling;
}


std::vector<winsys::Window> const&
StackHandler::get_layer(StackLayer layer) const
{
    switch (layer) {
    case StackLayer::Desktop:      return m_desktop;      break;
    case StackLayer::Below_:       return m_below;        break;
    case StackLayer::Dock:         return m_dock;         break;
    case StackLayer::Above_:       return m_above;        break;
    case StackLayer::Notification: return m_notification; break;
    default: Util::die("no associated layer");
    }

    return get_layer(StackLayer::Desktop);
}
