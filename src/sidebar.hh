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
        : m_enabled(USER_WORKSPACES.size(), true),
          m_ewmh(ewmh),
          m_context(nullptr),
          m_sidebarwin(x_data::create_window(true)),
          m_unmappedsidebarindicator(x_data::create_window(true)),
          m_layoutsymbolgc(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_contextlettergc(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_workspacenumbersgc(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_iconsgc(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_iconnumbersgc(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_numberstickygc(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_numberclientsgc(m_sidebarwin, FONTNAME, SIDEBAR_WIDTH),
          m_rootworkspacenumbergc(m_unmappedsidebarindicator, FONTNAME, SIDEBAR_WIDTH),
          m_activity_indicators(USER_WORKSPACES.size()),
          m_moveresizeindicator(x_data::create_window(true)),
          m_floatingindicator(x_data::create_window(true)),
          m_fullscreenindicator(x_data::create_window(true)),
          m_aboveindicator(x_data::create_window(true)),
          m_belowindicator(x_data::create_window(true))
    {
        auto root_attrs = x_data::get_attributes(x_data::g_root);
        m_sidebarwin.set_background_color(SIDEBAR_BG_COLOR);
        m_sidebarwin.set_border_color(SIDEBAR_BG_COLOR);
        m_sidebarwin.resize({SIDEBAR_WIDTH, root_attrs.get().height}).move({0, 0});
        x_data::select_input(m_sidebarwin, ExposureMask);

        m_unmappedsidebarindicator.set_background_color(USIND_BG_COLOR);
        m_unmappedsidebarindicator.set_border_color(ROOT_WORKSPACE_COLOR);
        m_unmappedsidebarindicator.resize({SIDEBAR_WIDTH, m_rootworkspacenumbergc.get_font_dim().h + 5}).move({0, 0});
        x_data::select_input(m_unmappedsidebarindicator, ExposureMask);

        m_ewmh.set_strut_property(m_sidebarwin, SIDEBAR_WIDTH + 2, 0, 0, 0);
        m_ewmh.set_window_type_property(m_sidebarwin, "DOCK");
        m_ewmh.set_window_type_property(m_unmappedsidebarindicator, "DOCK");

        m_ewmh.set_wm_name_property(m_sidebarwin, WMNAME);
        m_ewmh.set_class_property(m_sidebarwin, WMNAME);
        m_ewmh.set_supporting_wm_check_property(x_data::g_root, m_sidebarwin);
        m_ewmh.set_supporting_wm_check_property(m_sidebarwin, m_sidebarwin);
        m_ewmh.set_wm_pid_property(m_sidebarwin, getpid());

        m_unmappedsidebarindicator.map();
        m_sidebarwin.map();

        m_layoutsymbolgc.set_foreground(SIDEBAR_LAYOUT_COLOR);
        m_layoutsymbolgc.set_background(SIDEBAR_BG_COLOR);
        m_contextlettergc.set_foreground(SIDEBAR_CONTEXT_COLOR);
        m_contextlettergc.set_background(SIDEBAR_BG_COLOR);
        m_workspacenumbersgc.set_foreground(SIDEBAR_WORKSPACES_COLOR);
        m_workspacenumbersgc.set_background(SIDEBAR_BG_COLOR);
        m_iconsgc.set_foreground(SIDEBAR_ICONS_COLOR);
        m_iconsgc.set_background(SIDEBAR_BG_COLOR);
        m_iconnumbersgc.set_foreground(SIDEBAR_ICONNUMBERS_COLOR);
        m_iconnumbersgc.set_background(SIDEBAR_BG_COLOR);
        m_numberstickygc.set_foreground(SIDEBAR_NSTICKY_COLOR);
        m_numberstickygc.set_background(SIDEBAR_BG_COLOR);
        m_numberclientsgc.set_foreground(SIDEBAR_NCLIENTS_COLOR);
        m_numberclientsgc.set_background(SIDEBAR_BG_COLOR);
        m_rootworkspacenumbergc.set_foreground(ROOT_WORKSPACE_COLOR);
        m_rootworkspacenumbergc.set_background(USIND_BG_COLOR);

        pos_t current_pos = {SIDEBAR_WIDTH - 3, 2 * (4 + m_workspacenumbersgc.get_font_dim().h) + 4};
        for (::std::size_t i = 0; i < m_activity_indicators.size(); ++i) {
            m_activity_indicators[i] = x_data::create_window(true);
            m_ewmh.set_window_type_property(m_activity_indicators[i], "DOCK");
            m_activity_indicators[i].set_border_color(SIDEBAR_WORKSPACES_COLOR);
            m_activity_indicators[i].resize({1, 1}).reparent(current_pos, m_sidebarwin);
            current_pos.y += (2 + 1.25f * m_workspacenumbersgc.get_font_dim().h);
        }

        m_ewmh.set_window_type_property(m_moveresizeindicator, "DOCK");
        m_ewmh.set_window_type_property(m_floatingindicator, "DOCK");
        m_ewmh.set_window_type_property(m_fullscreenindicator, "DOCK");

        m_moveresizeindicator.set_background_color(MRIND_BORDER_COLOR);
        m_moveresizeindicator.set_border_width(0);
        m_moveresizeindicator.resize({2, 5}).reparent({root_attrs.w() - 3, 1}, m_sidebarwin);

        m_floatingindicator.set_background_color(FLIND_BG_COLOR).set_border_color(FLIND_BORDER_COLOR);
        m_floatingindicator.resize({1, 1}).reparent({SIDEBAR_WIDTH - 3, 1}, m_sidebarwin);

        m_fullscreenindicator.set_background_color(FSIND_BG_COLOR).set_border_color(FSIND_BORDER_COLOR);
        m_fullscreenindicator.resize({1, 1}).reparent({SIDEBAR_WIDTH - 3, 1}, m_sidebarwin);

        m_aboveindicator.set_background_color(ASIND_BG_COLOR).set_border_color(ASIND_BORDER_COLOR);
        m_aboveindicator.resize({1, 1}).reparent({SIDEBAR_WIDTH - 3, 1}, m_sidebarwin);

        m_belowindicator.set_background_color(BSIND_BG_COLOR).set_border_color(BSIND_BORDER_COLOR);
        m_belowindicator.resize({1, 1}).reparent({SIDEBAR_WIDTH - 3, 1}, m_sidebarwin);
    }

    void set_context(context_ptr_t);

    void draw();
    void toggle();
    void toggle_all();

    void draw_layoutsymbol();
    void draw_contextletter();
    void draw_clientstate();
    void draw_workspacenumbers();
    void draw_icons();
    void draw_numbersticky();
    void draw_numberclients();

    x_data::window_t get_win() const;
    x_data::window_t get_unmappedsidebarwin() const;

private:
    void map_sidebar();
    void unmap_sidebar();

    ::std::vector<bool> m_enabled;
    ewmh_t& m_ewmh;
    context_ptr_t m_context;

    x_data::window_t m_sidebarwin;
    x_data::window_t m_unmappedsidebarindicator;

    x_data::graphicscontext_t m_layoutsymbolgc;
    x_data::graphicscontext_t m_contextlettergc;
    x_data::graphicscontext_t m_workspacenumbersgc;
    x_data::graphicscontext_t m_iconsgc;
    x_data::graphicscontext_t m_iconnumbersgc;
    x_data::graphicscontext_t m_numberstickygc;
    x_data::graphicscontext_t m_numberclientsgc;
    x_data::graphicscontext_t m_rootworkspacenumbergc;

    ::std::vector<x_data::window_t> m_activity_indicators;
    x_data::window_t m_moveresizeindicator;
    x_data::window_t m_floatingindicator;
    x_data::window_t m_fullscreenindicator;
    x_data::window_t m_aboveindicator;
    x_data::window_t m_belowindicator;

};


#endif//__KRANEWM_SIDEBAR_GUARD__
