#ifndef __KRANEWM_SIDEBAR_GUARD__
#define __KRANEWM_SIDEBAR_GUARD__

#include "common.hh"
#include "decoration.hh"
#include "ewmh.hh"
#include "layout.hh"
#include "workspace.hh"
#include "x-wrapper/window.hh"
#include "x-wrapper/attributes.hh"
#include "x-wrapper/graphics.hh"

#include <vector>


class sidebar_t
{
public:
    explicit sidebar_t(ewmh_t& ewmh)
        : m_ewmh(ewmh),
          m_sidebarwin(x_wrapper::create_window(true)),
          m_graphicscontext(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_layoutsymbol(layout_t::floating),
          m_workspacenumber(0),
          m_numberclients(0),
          m_activity_indicators(USER_WORKSPACES.size()),
          m_workspace_activity(USER_WORKSPACES.size(), 0u),
          m_workspace_urgent(USER_WORKSPACES.size(), 0u)
    {
        auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
        m_sidebarwin.set_background_color(SIDEBAR_BG_COLOR);
        m_sidebarwin.resize({SIDEBAR_WIDTH, root_attrs.get().height}).move({0, 0});
        x_wrapper::select_input(m_sidebarwin, ExposureMask);

        m_ewmh.set_strut_property(m_sidebarwin, SIDEBAR_WIDTH + 2, 0, 0, 0);
        m_ewmh.set_window_type_property(m_sidebarwin, "DOCK");

        m_ewmh.set_wm_name_property(m_sidebarwin, WMNAME);
        m_ewmh.set_supporting_wm_check_property(x_wrapper::g_root, m_sidebarwin);
        m_ewmh.set_supporting_wm_check_property(m_sidebarwin, m_sidebarwin);

        m_sidebarwin.map();

        m_graphicscontext.set_foreground(SIDEBAR_FG_COLOR);
        m_graphicscontext.set_background(SIDEBAR_BG_COLOR);

        for (size_t i = 0; i < m_activity_indicators.size(); ++i) {
            m_activity_indicators[i] = x_wrapper::create_window(true);
            m_ewmh.set_window_type_property(m_activity_indicators[i], "INDICATOR");
            m_activity_indicators[i].set_border_color(SIDEBAR_WORKSPACES_COLOR);
            m_activity_indicators[i].resize({1, 1}).move({(SIDEBAR_WIDTH - 2),
                static_cast<int>((1.4f + i) * (4 + m_graphicscontext.get_font_dim().h))});
        }
    }

    void draw();

    sidebar_t& set_layoutsymbol(layout_t);
    sidebar_t& set_workspacenumber(unsigned);
    sidebar_t& set_numberclients(unsigned);

    sidebar_t& record_activity(unsigned);
    sidebar_t& erase_activity(unsigned);

    sidebar_t& record_urgent(unsigned);
    sidebar_t& erase_urgent(unsigned);

    x_wrapper::window_t get_win() const;

private:
    void draw_layoutsymbol();
    void draw_workspacenumber();
    void draw_numberclients();

    ewmh_t& m_ewmh;
    x_wrapper::window_t m_sidebarwin;
    x_wrapper::graphicscontext_t m_graphicscontext;

    layout_t m_layoutsymbol;
    unsigned m_workspacenumber;
    unsigned m_numberclients;

    ::std::vector<x_wrapper::window_t> m_activity_indicators;
    ::std::vector<int> m_workspace_activity;
    ::std::vector<int> m_workspace_urgent;

};


#endif//__KRANEWM_SIDEBAR_GUARD__
