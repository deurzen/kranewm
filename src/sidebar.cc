#include "sidebar.hh"

#include "context.hh"


void
sidebar_t::set_context(context_ptr_t context)
{
    m_context = context;
}

void
sidebar_t::draw()
{
    if (!m_enabled)
        return;

    m_graphicscontext.clear();
    draw_layoutsymbol();
    draw_workspacenumbers();
    draw_numbersticky();
    draw_numberclients();
}

void
sidebar_t::toggle()
{
    m_enabled = !m_enabled;
    if (m_enabled) {
        m_sidebarwin.map();
        m_ewmh.set_strut_property(m_sidebarwin, SIDEBAR_WIDTH + 2, 0, 0, 0);
        m_ewmh.check_apply_strut(m_sidebarwin);
        for (auto& ind : m_activity_indicators)
            ind.map();
        if (m_context->get_activated()->get_focused()
            && m_context->get_activated()->get_focused()->floating)
        {
            m_floatingindicator.map();
        } else if (m_context->get_activated()->get_focused()
            && m_context->get_activated()->get_focused()->fullscreen)
        {
            m_fullscreenindicator.map();
        }
    } else {
        m_sidebarwin.unmap();
        m_ewmh.check_release_strut(m_sidebarwin);
        m_moveresizeindicator.unmap();
        m_floatingindicator.unmap();
        m_fullscreenindicator.unmap();
        for (auto& ind : m_activity_indicators)
            ind.unmap();
    }
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

    m_graphicscontext.draw_string(pos,
        ::std::string(1, static_cast<char>(m_context->get_activated()->get_layout())));
}

void
sidebar_t::draw_workspacenumbers()
{
    m_graphicscontext.set_foreground(SIDEBAR_WORKSPACES_COLOR);
    pos_t current_pos = {(SIDEBAR_WIDTH - m_graphicscontext.get_font_dim().w) / 2,
        2 * (4 + m_graphicscontext.get_font_dim().h)};

    for (size_t i = 0; i < m_context->get_workspaces().size(); ++i) {
        if (m_context->get_nnonsticky(i)) {
            m_sidebarwin.lower();
            m_activity_indicators[i].map();
        } else
            m_activity_indicators[i].unmap();
    }

    for (auto& [nr,_] : USER_WORKSPACES) {
        if (nr == m_context->get_activated()->get_number()) {
            unsigned long color = m_context->get_workspaces()[nr - 1]->is_urgent()
                ? URG_COLOR
                : SIDEBAR_ACTIVE_WORKSPACE_COLOR;

            m_graphicscontext.set_foreground(SIDEBAR_ACTIVE_WORKSPACE_COLOR);
            m_activity_indicators[nr - 1].set_border_color(color);
            m_activity_indicators[nr - 1].set_background_color(color);
        } else {
            unsigned long color = m_context->get_workspaces()[nr - 1]->is_urgent()
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
sidebar_t::draw_numbersticky()
{
    m_graphicscontext.set_foreground(SIDEBAR_NSTICKY_COLOR);

    x_data::attributes_t root_attrs = x_data::get_attributes(x_data::g_root);
    pos_t pos = {(SIDEBAR_WIDTH - m_graphicscontext.get_font_dim().w) / 2,
        root_attrs.h() - 2 * (m_graphicscontext.get_font_dim().h) - 2};

    unsigned nsticky = m_context->get_nsticky();
    if (nsticky == 0)
        m_graphicscontext.draw_string(pos, " ");
    else if (nsticky > 9)
        m_graphicscontext.draw_string(pos, ">");
    else
        m_graphicscontext.draw_string(pos, ::std::to_string(nsticky));
}

void
sidebar_t::draw_numberclients()
{
    m_graphicscontext.set_foreground(SIDEBAR_NCLIENTS_COLOR);

    x_data::attributes_t root_attrs = x_data::get_attributes(x_data::g_root);
    pos_t pos = {(SIDEBAR_WIDTH - m_graphicscontext.get_font_dim().w) / 2,
        root_attrs.h() - (m_graphicscontext.get_font_dim().h) + 2};

    unsigned nclients = m_context->get_nnonsticky(m_context->get_activated()->get_number() - 1);
    if (nclients > 9)
        m_graphicscontext.draw_string(pos, ">");
    else
        m_graphicscontext.draw_string(pos, ::std::to_string(nclients));
}
