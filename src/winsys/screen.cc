#include "screen.hh"
#include "util.hh"

using namespace winsys;

std::vector<Window>
Screen::show_and_get_struts(bool show)
{
    m_showing_struts = show;
    compute_placeable_region();

    std::vector<Window> windows;
    windows.reserve(m_windows.size());

    std::transform(
        m_windows.begin(),
        m_windows.end(),
        std::back_inserter(windows),
        [](auto kv) -> Window {
            return kv.first;
        }
    );

    return windows;
}

void
Screen::add_struts(std::vector<std::optional<Strut>> struts)
{
    if (struts[0]) {
        add_strut(Edge::Left, *struts[0]);
    }

    if (struts[1]) {
        add_strut(Edge::Top, *struts[1]);
    }

    if (struts[2]) {
        add_strut(Edge::Right, *struts[2]);
    }

    if (struts[3]) {
        add_strut(Edge::Bottom, *struts[3]);
    }
}

void
Screen::add_strut(Edge edge, Strut strut)
{
    m_struts.at(edge).insert(strut);

    if (m_windows.count(strut.window) > 0) {
        std::vector<Edge>& edges = m_windows.at(strut.window);
        edges.push_back(edge);
    } else
        m_windows[strut.window] = { edge };
}

void
Screen::add_strut(Edge edge, Window window, int width)
{
    m_struts.at(edge).insert(Strut {
        window,
        width
    });

    if (m_windows.count(window) > 0) {
        std::vector<Edge>& edges = m_windows.at(window);
        edges.push_back(edge);
    } else
        m_windows[window] = { edge };
}

void
Screen::update_strut(Edge edge, Window window, int width)
{
    remove_strut(window);
    add_strut(edge, window, width);
}

void
Screen::remove_strut(Window window)
{
    std::optional<std::vector<Edge>> const& edges
        = Util::retrieve(m_windows, window);

    if (edges)
        for (auto& edge : *edges) {
            std::set<Strut, StrutComparer>& struts = m_struts.at(edge);

            auto iter = std::find_if(
                struts.begin(),
                struts.end(),
                [window](Strut const& strut) -> bool {
                    return strut.window == window;
                }
            );

            if (iter != struts.end())
                struts.erase(iter);
        }

    m_windows.erase(window);
}

void
Screen::compute_placeable_region()
{
    Region region = m_full_region;

    if (m_showing_struts) {
        std::set<Strut, StrutComparer>& left = m_struts.at(Edge::Left);
        std::set<Strut, StrutComparer>& top = m_struts.at(Edge::Top);
        std::set<Strut, StrutComparer>& right = m_struts.at(Edge::Right);
        std::set<Strut, StrutComparer>& bottom = m_struts.at(Edge::Bottom);

        if (!left.empty()) {
            int width = left.rbegin()->width;
            region.pos.x += width;
            region.dim.w -= width;
        }

        if (!top.empty()) {
            int width = top.rbegin()->width;
            region.pos.y += width;
            region.dim.h -= width;
        }

        if (!right.empty()) {
            int width = right.rbegin()->width;
            region.dim.w -= width;
        }

        if (!bottom.empty()) {
            int width = bottom.rbegin()->width;
            region.dim.h -= width;
        }
    }

    m_placeable_region = region;
}
