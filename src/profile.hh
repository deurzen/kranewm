#ifndef __KRANEWM_PROFILE_GUARD__
#define __KRANEWM_PROFILE_GUARD__

#include "workspace.hh"

#include "x-data/common.hh"

#include <vector>


class profile_t
{
public:
    profile_t() = default;

    void save(const user_workspace_ptr_t);
    void apply(const user_workspace_ptr_t);

private:
    ::std::vector<::std::pair<pos_t, dim_t>> m_geometries;

    ::std::size_t m_nmaster;
    ::std::size_t m_gap_size;
    float         m_mfactor;
    layout_t      m_layout;
    bool          m_mirrored;

};

#endif//__KRANEWM_PROFILE_GUARD__
