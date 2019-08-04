#include "profile.hh"

#include <algorithm>


void
profile_t::save(const ::std::deque<client_ptr_t>& clients)
{
    m_geometries.clear();
    for (auto& client : clients)
        m_geometries.emplace_back(client->float_pos, client->float_dim);
}

void
profile_t::apply(const ::std::deque<client_ptr_t>& clients)
{
    for (size_t i = 0; i < ::std::min(m_geometries.size(), clients.size()); ++i) {
        clients[i]->float_pos = m_geometries[i].first;
        clients[i]->float_dim = m_geometries[i].second;
    }
}
