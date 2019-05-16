#include "sidebar.hh"


void
sidebar_t::draw()
{
    if (!SHOW_SIDEBAR)
        return;

    m_graphicscontext.clear();
    draw_layoutsymbol();
    draw_workspacenumber();
    draw_numberclients();
}

sidebar_t&
sidebar_t::set_layoutsymbol(layout_t layout)
{
    m_layoutsymbol = layout;
    return *this;
}

sidebar_t&
sidebar_t::set_workspacenumber(unsigned workspace)
{
    m_workspacenumber = workspace;
    return *this;
}

sidebar_t&
sidebar_t::set_numberclients(unsigned n)
{
    m_numberclients = n;
    return *this;
}

sidebar_t&
sidebar_t::record_activity(unsigned workspace)
{
    ++m_workspace_activity[workspace - 1];
    return *this;
}

sidebar_t&
sidebar_t::erase_activity(unsigned workspace)
{
    if (m_workspace_activity[workspace - 1] > 0)
        --m_workspace_activity[workspace - 1];
    return *this;
}

sidebar_t&
sidebar_t::record_urgent(unsigned workspace)
{
    ++m_workspace_urgent[workspace - 1];
    return *this;
}

sidebar_t&
sidebar_t::erase_urgent(unsigned workspace)
{
    if (m_workspace_urgent[workspace - 1] > 0)
        --m_workspace_urgent[workspace - 1];
    return *this;
}

sidebar_t&
sidebar_t::indicate_moveresize()
{
    m_moveresizeindicator.map();
    return *this;
}

sidebar_t&
sidebar_t::indicate_nomoveresize()
{
    m_moveresizeindicator.unmap();
    return *this;
}

sidebar_t&
sidebar_t::indicate_clientfullscreen()
{
    m_floatingindicator.unmap();
    m_fullscreenindicator.map();
    return *this;
}

sidebar_t&
sidebar_t::indicate_clientfloating()
{
    m_fullscreenindicator.unmap();
    m_floatingindicator.map();
    return *this;
}

sidebar_t&
sidebar_t::indicate_clientnormal()
{

    m_floatingindicator.unmap();
    m_fullscreenindicator.unmap();
    return *this;
}

x_data::window_t
sidebar_t::get_win() const
{
    return m_sidebarwin;
}


void
sidebar_t::draw_layoutsymbol()
{
    m_graphicscontext.set_foreground(SIDEBAR_LAYOUT_COLOR);

    pos_t pos = {(SIDEBAR_WIDTH - m_graphicscontext.get_font_dim().w) / 2,
        4 + m_graphicscontext.get_font_dim().h};

    m_graphicscontext.draw_string(pos, ::std::string(1, static_cast<char>(m_layoutsymbol)));
}

void
sidebar_t::draw_workspacenumber()
{
    m_graphicscontext.set_foreground(SIDEBAR_WORKSPACES_COLOR);
    pos_t current_pos = {(SIDEBAR_WIDTH - m_graphicscontext.get_font_dim().w) / 2,
        2 * (4 + m_graphicscontext.get_font_dim().h)};

    for (size_t i = 0; i < m_activity_indicators.size(); ++i) {
        if (m_workspace_activity[i]) {
            m_sidebarwin.lower();
            m_activity_indicators[i].map();
        } else
            m_activity_indicators[i].unmap();
    }

    for (auto& [nr,_] : USER_WORKSPACES) {
        if (nr == m_workspacenumber) {
            unsigned long color = m_workspace_urgent[nr - 1]
                ? URG_COLOR
                : SIDEBAR_ACTIVE_WORKSPACE_COLOR;

            m_graphicscontext.set_foreground(SIDEBAR_ACTIVE_WORKSPACE_COLOR);
            m_activity_indicators[nr - 1].set_border_color(color);
            m_activity_indicators[nr - 1].set_background_color(color);
        } else {
            unsigned long color = m_workspace_urgent[nr - 1]
                ? URG_COLOR
                : SIDEBAR_WORKSPACES_COLOR;

            m_graphicscontext.set_foreground(SIDEBAR_WORKSPACES_COLOR);
            m_activity_indicators[nr - 1].set_border_color(color);
            m_activity_indicators[nr - 1].set_background_color(color);
        }

        m_graphicscontext.draw_string(current_pos, ::std::to_string(nr));
        current_pos.y += (4 + m_graphicscontext.get_font_dim().h);
    }
}

void
sidebar_t::draw_numberclients()
{
    m_graphicscontext.set_foreground(SIDEBAR_NCLIENTS_COLOR);

    x_data::attributes_t root_attrs = x_data::get_attributes(x_data::g_root);
    pos_t pos = {(SIDEBAR_WIDTH - m_graphicscontext.get_font_dim().w) / 2,
        root_attrs.h() - (m_graphicscontext.get_font_dim().h)};

    if (m_numberclients > 9)
        m_graphicscontext.draw_string(pos, ">");
    else
        m_graphicscontext.draw_string(pos, ::std::to_string(m_numberclients));
}
