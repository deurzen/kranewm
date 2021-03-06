#ifndef __KRANEWM_PROFILE_GUARD__
#define __KRANEWM_PROFILE_GUARD__

#include "workspace.hh"

#include "x-data/common.hh"

#include <vector>


class profile_t final
{
public:
    profile_t() = default;

    bool
    is_set() const
    {
        return m_set;
    }

    void save(const user_workspace_ptr_t);
    void apply(const user_workspace_ptr_t);

private:
    ::std::vector<::std::pair<pos_t, dim_t>> m_geometries;

    bool          m_set = false;
    ::std::size_t m_nmaster;
    ::std::size_t m_gapsize;
    float         m_mfactor;
    layout_t      m_layout;
    bool          m_mirrored;

};

#endif//__KRANEWM_PROFILE_GUARD__
