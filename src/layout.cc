#include "layout.hh"
#include "ewmh.hh"
#include "workspace.hh"
#include "x-data/attributes.hh"

#include <algorithm>
#include <functional>
#include <cmath>


void
layoutzone_t::arrange(const user_workspace_t& workspace,
    ::std::pair<::std::deque<client_ptr_t>::const_iterator, ::std::deque<client_ptr_t>::const_iterator> range) const
{
    auto gap_size = workspace.get_gap_size();
    auto size = range.second - range.first;

    if (size <= 0)
        return;

    pos_t current_pos = m_pos;
    dim_t current_dim = m_dim;

    switch (m_type) {
    case zonetype_t::htile: current_dim = {m_dim.h, m_dim.w / size}; break;
    case zonetype_t::vtile: current_dim = {m_dim.h / size, m_dim.w}; break;
    default: break;
    }

    while (range.first != range.second) {
        (*range.first)->resize(current_dim, true).move(current_pos, true);

        switch (m_type) {
        case zonetype_t::htile: current_pos.x += current_dim.w; break;
        case zonetype_t::vtile: current_pos.y += current_dim.h; break;
        default: break;
        }

        ++range.first;
    }
}

bool
layoutsetting_t::evaluate(const user_workspace_t& workspace) const
{
    ::std::size_t nmaster = ::std::min(workspace.get_all().size(), workspace.get_nmaster());
    ::std::size_t nstack = workspace.get_all().size() - nmaster;

    return m_nclients_op(workspace.get_all().size(), m_nclients)
        && m_nmaster_op(nmaster, m_nmaster)
        && m_nstack_op(nstack, m_nstack);
}


void
layoutsetting_t::apply(const user_workspace_t& workspace)
{
    { // arrange master zones
        auto nmasterzones = m_masterzones.size();
        auto nmaster = ::std::min(workspace.get_all().size(), workspace.get_nmaster());
        ::std::vector<::std::size_t> zone_nclients;

        for (::std::size_t i = 0; i < m_masterzones.size(); ++i)
            zone_nclients.push_back(::std::floor(nmaster / nmasterzones));

        ::std::size_t rem = nmaster - m_masterzones.size() * ::std::floor(nmaster / nmasterzones);

        for (::std::size_t i = 0; i < rem; ++i)
            ++zone_nclients[i];

        auto it = workspace.get_all().begin();
        for (::std::size_t i = 0; i < m_masterzones.size(); ++i) {
            m_masterzones[i]->arrange(workspace, ::std::pair{it, it + zone_nclients[i]});
            it += zone_nclients[i] - 1;
        }
    }

    { // arrange stack zones
        auto nstackzones = m_stackzones.size();
        auto nstack = ::std::max(static_cast<::std::size_t>(0),
            workspace.get_all().size() - workspace.get_nmaster());
        ::std::vector<::std::size_t> zone_nclients;

        for (::std::size_t i = 0; i < m_stackzones.size(); ++i)
            zone_nclients.push_back(::std::floor(nstack / nstackzones));

        ::std::size_t rem = nstack - m_stackzones.size() * ::std::floor(nstack / nstackzones);

        for (::std::size_t i = 0; i < rem; ++i)
            ++zone_nclients[i];

        auto it = workspace.get_all().begin() + workspace.get_nmaster();
        for (::std::size_t i = 0; i < m_stackzones.size(); ++i) {
            m_stackzones[i]->arrange(workspace, ::std::pair{it, it + zone_nclients[i]});
            it += zone_nclients[i] - 1;
        }
    }
}

void
layouthandler_t::add_tile_layout()
{
    auto root_attrs = x_data::get_attributes(x_data::g_root);
    pos_t zone_origin = {m_ewmh.get_left_strut(), m_ewmh.get_top_strut()};
    dim_t zone_area   = {root_attrs.w() - zone_origin.x - m_ewmh.get_right_strut(),
        root_attrs.h() - zone_origin.y - m_ewmh.get_bottom_strut()};

    layout_t layout = layout_t{layouttype_t::tile};

    { // nmaster = 0 or nstack = 0 setting
        auto setting_nmaster_0 = new layoutsetting_t{
            layoutsetting_t::settingtarget_t::nmaster, 0, layoutsetting_t::equal_to};

        auto setting_nstack_0 = new layoutsetting_t{
            layoutsetting_t::settingtarget_t::nstack, 0, layoutsetting_t::equal_to};

        layout.add_setting(setting_nmaster_0);
        layout.add_setting(setting_nstack_0);

        { // master zone
            auto zone = new layoutzone_t{layoutzone_t::zonetype_t::vtile, zone_origin, zone_area};
            setting_nmaster_0->add_masterzone(zone);
            setting_nstack_0->add_stackzone(zone);
        }
    }

    { // default setting
        auto setting_default = new layoutsetting_t{};

        layout.add_setting(setting_default);

        { // master zone
            auto zone = new layoutzone_t{layoutzone_t::zonetype_t::vtile,
                zone_origin, {zone_area.w / 2, zone_area.h}};
            setting_default->add_masterzone(zone);
        }

        { // stack zone
            auto zone = new layoutzone_t{layoutzone_t::zonetype_t::vtile,
                {zone_origin.x + zone_area.w / 2, zone_origin.y}, {zone_area.w / 2, zone_area.h}};
            setting_default->add_stackzone(zone);
        }
    }

    m_layouts[layouttype_t::tile] = layout;
}

void
layouthandler_t::add_stick_layout()
{

}

void
layouthandler_t::add_deck_layout()
{

}

void
layouthandler_t::add_doubledeck_layout()
{

}

void
layouthandler_t::add_grid_layout()
{

}

void
layouthandler_t::add_pillar_layout()
{

}

void
layouthandler_t::add_column_layout()
{

}

void
layouthandler_t::add_monocle_layout()
{

}

void
layouthandler_t::add_center_layout()
{

}
