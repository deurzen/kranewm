#include "sidebar.hh"

#include "context.hh"
#include "iconification.hh"

#include <algorithm>


void
sidebar_t::set_context(context_ptr_t context)
{
    m_context = context;
}

void
sidebar_t::draw()
{
    if (!m_enabled.at(m_context->get_activated()->get_index())) {
        unmap_sidebar();
        return;
    } else
        map_sidebar();

    draw_layoutsymbol();
    draw_contextletter();
    draw_clientstate();
    draw_workspacenumbers();
    draw_icons();
    draw_numbersticky();
    draw_numberclients();
}

void
sidebar_t::toggle()
{
    bool is_enabled = m_enabled.at(m_context->get_activated()->get_index());
    m_enabled[m_context->get_activated()->get_index()] = !is_enabled;
    is_enabled = m_enabled.at(m_context->get_activated()->get_index());

    if (is_enabled)
        map_sidebar();
    else
        unmap_sidebar();

    m_context->get_activated()->sidebarset(is_enabled);
}

void
sidebar_t::toggle_all()
{
    bool is_enabled = m_enabled.at(m_context->get_activated()->get_index());

    for (size_t i = 0; i < USER_WORKSPACES.size(); ++i) {
        m_enabled[i] = !is_enabled;

        if (m_enabled.at(i))
            map_sidebar();
        else
            unmap_sidebar();
    }
}

x_data::window_t
sidebar_t::get_win() const
{
    return m_sidebarwin;
}

x_data::window_t
sidebar_t::get_unmappedsidebarwin() const
{
    return m_unmappedsidebarindicator;
}


void
sidebar_t::draw_layoutsymbol()
{
    pos_t pos = {(SIDEBAR_WIDTH - m_layoutsymbolgc.get_font_dim().w) / 2,
        4 + m_layoutsymbolgc.get_font_dim().h};

    m_layoutsymbolgc.clear({pos.x, pos.y - 4});
    m_layoutsymbolgc.clear({pos.x, pos.y + 4});
    m_layoutsymbolgc.clear();
    m_layoutsymbolgc.draw_string(pos,
        ::std::string(1, static_cast<char>(m_context->get_activated()->get_layout())));
}

void
sidebar_t::draw_contextletter()
{
    pos_t pos = {(SIDEBAR_WIDTH - m_contextlettergc.get_font_dim().w) / 2,
        2 * (4 + m_contextlettergc.get_font_dim().h)};

    m_contextlettergc.clear({pos.x, pos.y - 4});
    m_contextlettergc.clear({pos.x, pos.y + 4});
    m_contextlettergc.clear();
    m_contextlettergc.draw_string(pos, ::std::string(1, m_context->get_letter()));
}

void
sidebar_t::draw_clientstate()
{
    auto client = m_context->get_activated()->get_focused();
    if (client && client->fullscreen) {
        m_fullscreenindicator.map();
        m_floatingindicator.unmap();
        m_aboveindicator.unmap();
        m_belowindicator.unmap();
    } else if (client && client->above) {
        m_aboveindicator.map();
        m_floatingindicator.unmap();
        m_fullscreenindicator.unmap();
        m_belowindicator.unmap();
    } else if (client && client->below) {
        m_belowindicator.map();
        m_floatingindicator.unmap();
        m_fullscreenindicator.unmap();
        m_aboveindicator.unmap();
    } else if (client && client->floating) {
        m_floatingindicator.map();
        m_fullscreenindicator.unmap();
        m_aboveindicator.unmap();
        m_belowindicator.unmap();
    } else {
        m_floatingindicator.unmap();
        m_fullscreenindicator.unmap();
        m_aboveindicator.unmap();
        m_belowindicator.unmap();
    }
}

void
sidebar_t::draw_workspacenumbers()
{
    pos_t current_pos = {(SIDEBAR_WIDTH - m_workspacenumbersgc.get_font_dim().w) / 2,
        3 * (4 + m_workspacenumbersgc.get_font_dim().h)};

    for (::std::size_t i = 0; i < (*m_context->get_workspaces()).size(); ++i) {
        if (m_context->get_nnonsticky(i))
            m_activity_indicators[i].map();
        else
            m_activity_indicators[i].unmap();
    }

    for (auto& [nr,_] : USER_WORKSPACES) {
        if (nr == m_context->get_activated()->get_number()) {
            unsigned long color = (*m_context->get_workspaces())[nr - 1]->is_urgent()
                ? URG_COLOR
                : SIDEBAR_ACTIVE_WORKSPACE_COLOR;

            m_workspacenumbersgc.set_foreground(SIDEBAR_ACTIVE_WORKSPACE_COLOR);
            m_activity_indicators[nr - 1].set_border_color(color);
            m_activity_indicators[nr - 1].set_background_color(color);
        } else {
            unsigned long color = (*m_context->get_workspaces())[nr - 1]->is_urgent()
                ? URG_COLOR
                : SIDEBAR_WORKSPACES_COLOR;

            m_workspacenumbersgc.set_foreground(SIDEBAR_WORKSPACES_COLOR);
            m_activity_indicators[nr - 1].set_border_color(color);
            m_activity_indicators[nr - 1].set_background_color(color);
        }

        m_workspacenumbersgc.clear(current_pos);
        m_workspacenumbersgc.draw_string(current_pos, ::std::to_string(nr));
        current_pos.y += (2 + 1.25f * m_workspacenumbersgc.get_font_dim().h);
    }
}

void
sidebar_t::draw_icons()
{
    pos_t current_pos = {(SIDEBAR_WIDTH - m_workspacenumbersgc.get_font_dim().w) / 2,
        14 * (4 + m_workspacenumbersgc.get_font_dim().h)};

    auto icons = m_context->get_activated()->get_icons();
    for (::std::size_t i = 0; i < 16; ++i) {
        m_iconsgc.clear({current_pos.x,
            current_pos.y + static_cast<int>(i * (4 + m_workspacenumbersgc.get_font_dim().h)) - 2});
        m_iconsgc.clear({current_pos.x,
            current_pos.y + static_cast<int>(i * (4 + m_workspacenumbersgc.get_font_dim().h)) + 2});
    }

    for (::std::size_t i = 0; i < ::std::min(icons.size(), static_cast<::std::size_t>(10)); ++i) {
        if (i == 0 || i == 2 || i == 4 || i == 6 || i == 8) {
            m_iconnumbersgc.draw_string(current_pos, ::std::to_string(i + 1));
            current_pos.y += (4 + m_workspacenumbersgc.get_font_dim().h);
        } else if (i == 9) {
            for (::std::size_t j = 0; j < 3; ++j) {
                m_iconnumbersgc.draw_string({current_pos.x, current_pos.y - 5}, ".");
                current_pos.y += (m_workspacenumbersgc.get_font_dim().h - 4);
            }
            break;
        }

        ::std::string cls = icons[i]->win.get_class();
        char icon_char = ICON_CHARACTERS.at(applicationtype_t::other);
        if (APPLICATION_TYPES.count(cls))
            icon_char = ICON_CHARACTERS.at(APPLICATION_TYPES.at(cls));

        m_iconsgc.draw_string(current_pos, ::std::string(1, icon_char));
        current_pos.y += (4 + m_workspacenumbersgc.get_font_dim().h);
    }
}

void
sidebar_t::draw_numbersticky()
{
    x_data::attributes_t root_attrs = x_data::get_attributes(x_data::g_root);
    pos_t pos = {(SIDEBAR_WIDTH - m_numberstickygc.get_font_dim().w) / 2,
        root_attrs.h() - 2 * (m_numberstickygc.get_font_dim().h) - 2};

    ::std::size_t nsticky = m_context->get_nsticky();
    m_numberstickygc.clear();

    if (nsticky == 0)
        m_numberstickygc.draw_string(pos, " ");
    else if (nsticky > 9)
        m_numberstickygc.draw_string(pos, ">");
    else
        m_numberstickygc.draw_string(pos, ::std::to_string(nsticky));
}

void
sidebar_t::draw_numberclients()
{
    x_data::attributes_t root_attrs = x_data::get_attributes(x_data::g_root);
    pos_t pos = {(SIDEBAR_WIDTH - m_numberclientsgc.get_font_dim().w) / 2,
        root_attrs.h() - (m_numberclientsgc.get_font_dim().h) + 2};

    ::std::size_t nclients = m_context->get_nnonsticky(m_context->get_activated()->get_number() - 1);
    m_numberclientsgc.clear();

    if (nclients > 9)
        m_numberclientsgc.draw_string(pos, ">");
    else
        m_numberclientsgc.draw_string(pos, ::std::to_string(nclients));
}

void
sidebar_t::map_sidebar()
{
    m_sidebarwin.map();
    m_unmappedsidebarindicator.unmap();
    m_ewmh.set_strut_property(m_sidebarwin, SIDEBAR_WIDTH + 2, 0, 0, 0);
    m_ewmh.check_apply_strut(m_sidebarwin);
}

void
sidebar_t::unmap_sidebar()
{
    m_unmappedsidebarindicator.map();
    m_rootworkspacenumbergc.clear();
    m_rootworkspacenumbergc.draw_string({(SIDEBAR_WIDTH - m_numberstickygc.get_font_dim().w) / 2,
        m_rootworkspacenumbergc.get_font_dim().h + 2}, ::std::to_string(m_context->get_activated()->get_number()));
    m_sidebarwin.unmap();
    m_ewmh.check_release_strut(m_sidebarwin);
}
