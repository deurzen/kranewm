#include "profile.hh"

#include <algorithm>


void
profile_t::save(const user_workspace_ptr_t workspace)
{
    m_geometries.clear();
    for (auto& client : workspace->get_all())
        m_geometries.emplace_back(client->float_pos, client->float_dim);

    m_nmaster = workspace->get_nmaster();
    m_gap_size = workspace->get_gap_size();
    m_mfactor = workspace->get_mfactor();
    m_layout = workspace->get_layout();
    m_mirrored = workspace->is_mirrored();
}

void
profile_t::apply(const user_workspace_ptr_t workspace)
{
    auto clients = workspace->get_all();
    for (::std::size_t i = 0; i < ::std::min(m_geometries.size(), clients.size()); ++i) {
        clients[i]->float_pos = m_geometries[i].first;
        clients[i]->float_dim = m_geometries[i].second;
    }

    workspace->set_nmaster(m_nmaster);
    workspace->set_gap_size(m_gap_size);
    workspace->set_mfactor(m_mfactor);
    workspace->set_layout(m_layout);

    if (m_mirrored != workspace->is_mirrored())
        workspace->mirror();
}
