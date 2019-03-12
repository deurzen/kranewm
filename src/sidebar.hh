#ifndef __KRANEWM_SIDEBAR_GUARD__
#define __KRANEWM_SIDEBAR_GUARD__

#include "common.hh"
#include "decoration.hh"
#include "ewmh.hh"
#include "layout.hh"
#include "x-wrapper/window.hh"
#include "x-wrapper/attributes.hh"
#include "x-wrapper/graphics.hh"

enum textposition_t
{
    top_1,
    top_2,
    top_3,
    mid_1,
    bot_1,
    bot_2
};


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
          m_layoutsymbol_pos(top_1),
          m_workspacenumber_pos(top_2),
          m_numberclients_pos(bot_1)
    {
        auto root_attrs = x_wrapper::get_attributes(x_wrapper::g_root);
        m_sidebarwin.set_background_color(SIDEBAR_BG_COLOR);
        m_sidebarwin.resize({SIDEBAR_WIDTH, root_attrs.get().height}).move({0, 0});
        x_wrapper::select_input(m_sidebarwin, ExposureMask);

        m_ewmh.set_strut_property(m_sidebarwin, SIDEBAR_WIDTH, 0, 0, 0);
        m_ewmh.set_window_type_property(m_sidebarwin, "DESKTOP");

        m_ewmh.set_wm_name_property(m_sidebarwin, WMNAME);
        m_ewmh.set_supporting_wm_check_property(x_wrapper::g_root, m_sidebarwin);
        m_ewmh.set_supporting_wm_check_property(m_sidebarwin, m_sidebarwin);

        m_sidebarwin.map();

        m_graphicscontext.set_foreground(SIDEBAR_FG_COLOR);
        m_graphicscontext.set_background(SIDEBAR_BG_COLOR);
    }

    void draw();

    sidebar_t& set_layoutsymbol(layout_t);
    sidebar_t& set_workspacenumber(unsigned);
    sidebar_t& set_numberclients(unsigned);

    x_wrapper::window_t get_win() const;

private:
    pos_t get_position(textposition_t);

    void draw_layoutsymbol();
    void draw_workspacenumber();
    void draw_numberclients();

    ewmh_t& m_ewmh;
    x_wrapper::window_t m_sidebarwin;
    x_wrapper::graphicscontext_t m_graphicscontext;

    layout_t m_layoutsymbol;
    unsigned m_workspacenumber;
    unsigned m_numberclients;

    textposition_t m_layoutsymbol_pos;
    textposition_t m_workspacenumber_pos;
    textposition_t m_numberclients_pos;

};


#endif//__KRANEWM_SIDEBAR_GUARD__
