#ifndef __WINSYS_SCREEN_H_GUARD__
#define __WINSYS_SCREEN_H_GUARD__

#include "common.hh"
#include "geometry.hh"
#include "window.hh"
#include "util.hh"

#include <set>
#include <unordered_map>
#include <vector>

namespace winsys
{

    class Screen final
    {
    public:
        Screen(Index index, Region region)
            : m_index(index),
              m_full_region(region),
              m_placeable_region(region),
              m_windows({}),
              m_struts({
                  { Edge::Left,   {{}, s_strut_comparer} },
                  { Edge::Top,    {{}, s_strut_comparer} },
                  { Edge::Right,  {{}, s_strut_comparer} },
                  { Edge::Bottom, {{}, s_strut_comparer} },
              }),
              m_showing_struts(true)
        {}

        Region
        full_region() const
        {
            return m_full_region;
        }

        Region
        placeable_region() const
        {
            return m_placeable_region;
        }

        bool
        showing_struts() const
        {
            return m_showing_struts;
        }

        bool
        contains_strut(Window window) const
        {
            return m_windows.count(window) > 0;
        }

        Index
        index() const
        {
            return m_index;
        }

        void
        set_index(Index index)
        {
            m_index = index;
        }

        std::optional<int>
        max_strut_at_edge(winsys::Edge edge)
        {
            return m_struts.at(edge).empty()
                ? std::nullopt
                : std::optional(m_struts[edge].rbegin()->width);
        }

        std::vector<Window> show_and_get_struts(bool);

        void add_struts(std::vector<std::optional<Strut>>);
        void add_strut(Edge, Strut);
        void add_strut(Edge, Window, int);
        void update_strut(Edge, Window, int);
        void remove_strut(Window);

        void compute_placeable_region();

    private:
        Index m_index;

        Region m_full_region;
        Region m_placeable_region;

        static constexpr struct StrutComparer final {
            bool
            operator()(const Strut& lhs, const Strut& rhs) const
            {
                return lhs.width < rhs.width;
            }
        } s_strut_comparer{};

        std::unordered_map<Window, std::vector<Edge>> m_windows;
        std::unordered_map<Edge, std::set<Strut, StrutComparer>> m_struts;

        bool m_showing_struts;

    };

}

#endif//__WINSYS_SCREEN_H_GUARD__
