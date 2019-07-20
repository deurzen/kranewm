#ifndef __KRANEWM_SIDEBAR_GUARD__
#define __KRANEWM_SIDEBAR_GUARD__

#include "common.hh"
#include "decoration.hh"
#include "ewmh.hh"
#include "layout.hh"
#include "workspace.hh"

#include "x-data/window.hh"
#include "x-data/attributes.hh"
#include "x-data/graphics.hh"

#include <vector>

// fwd decls
typedef class context_t* context_ptr_t;

class sidebar_t
{
public:
    explicit sidebar_t(ewmh_t& ewmh)
        : m_enabled(true),
          m_ewmh(ewmh),
          m_sidebarwin(x_data::create_window(true)),
          m_graphicscontext(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_context(nullptr),
          m_activity_indicators(USER_WORKSPACES.size()),
          m_moveresizeindicator(x_data::create_window(true)),
          m_floatingindicator(x_data::create_window(true)),
          m_fullscreenindicator(x_data::create_window(true))
    {
        auto root_attrs = x_data::get_attributes(x_data::g_root);
        m_sidebarwin.set_background_color(SIDEBAR_BG_COLOR);
        m_sidebarwin.resize({SIDEBAR_WIDTH, root_attrs.get().height}).move({0, 0});
        x_data::select_input(m_sidebarwin, ExposureMask);

        m_ewmh.set_strut_property(m_sidebarwin, SIDEBAR_WIDTH + 2, 0, 0, 0);
        m_ewmh.set_window_type_property(m_sidebarwin, "DOCK");

        m_ewmh.set_wm_name_property(m_sidebarwin, WMNAME);
        m_ewmh.set_supporting_wm_check_property(x_data::g_root, m_sidebarwin);
        m_ewmh.set_supporting_wm_check_property(m_sidebarwin, m_sidebarwin);

        m_sidebarwin.map();

        m_graphicscontext.set_foreground(SIDEBAR_FG_COLOR);
        m_graphicscontext.set_background(SIDEBAR_BG_COLOR);

        for (size_t i = 0; i < m_activity_indicators.size(); ++i) {
            m_activity_indicators[i] = x_data::create_window(true);
            m_ewmh.set_window_type_property(m_activity_indicators[i], "DOCK");
            m_activity_indicators[i].set_border_color(SIDEBAR_WORKSPACES_COLOR);
            m_activity_indicators[i].resize({1, 1}).move({SIDEBAR_WIDTH - 2,
                static_cast<int>((2.4f + i) * (4 + m_graphicscontext.get_font_dim().h))});
        }

        m_ewmh.set_window_type_property(m_moveresizeindicator, "DOCK");
        m_ewmh.set_window_type_property(m_floatingindicator, "DOCK");
        m_ewmh.set_window_type_property(m_fullscreenindicator, "DOCK");

        m_moveresizeindicator.set_background_color(MRIND_BORDER_COLOR);
        m_moveresizeindicator.resize({2, 5}).move({root_attrs.w() - 3, 1}).set_border_width(0);

        m_floatingindicator.set_background_color(FLIND_BG_COLOR).set_border_color(FLIND_BORDER_COLOR);
        m_floatingindicator.resize({1, 1}).move({SIDEBAR_WIDTH - 2, 1});

        m_fullscreenindicator.set_background_color(FSIND_BG_COLOR).set_border_color(FSIND_BORDER_COLOR);
        m_fullscreenindicator.resize({1, 1}).move({SIDEBAR_WIDTH - 2, 1});
    }

    void set_context(context_ptr_t);

    void draw();
    void toggle();

    x_data::window_t get_win() const;

private:
    void draw_layoutsymbol();
    void draw_contextletter();
    void draw_clientstate();
    void draw_workspacenumbers();
    void draw_numbersticky();
    void draw_numberclients();

    bool m_enabled;
    ewmh_t& m_ewmh;
    x_data::window_t m_sidebarwin;
    x_data::graphicscontext_t m_graphicscontext;
    context_ptr_t m_context;

    ::std::vector<x_data::window_t> m_activity_indicators;
    x_data::window_t m_moveresizeindicator;
    x_data::window_t m_floatingindicator;
    x_data::window_t m_fullscreenindicator;

};


#endif//__KRANEWM_SIDEBAR_GUARD__
