#ifndef __KRANEWM_PROFILE_GUARD__
#define __KRANEWM_PROFILE_GUARD__

#include "client.hh"

#include "x-data/common.hh"

#include <vector>


class profile_t
{
public:
    profile_t() = default;

    void save(const ::std::deque<client_ptr_t>&);
    void apply(const ::std::deque<client_ptr_t>&);

private:
    ::std::vector<::std::pair<pos_t, dim_t>> m_geometries;

};

#endif//__KRANEWM_PROFILE_GUARD__
