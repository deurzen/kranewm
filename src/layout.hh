#ifndef __KRANEWM_LAYOUT_GUARD__
#define __KRANEWM_LAYOUT_GUARD__

#include "x-data/common.hh"

#include <deque>
#include <functional>
#include <limits>
#include <vector>

// fwd decls
class user_workspace_t;
class ewmh_t;
typedef class client_t* client_ptr_t;


enum class layouttype_t : char
{// name         symbol
    toggle     = '_',
    floating   = 'F',
    tile       = 'T',
    stick      = 'S',
    deck       = 'D',
    doubledeck = '$',
    grid       = '#',
    pillar     = 'P',
    column     = 'C',
    monocle    = 'M',
    center     = '^',
};

typedef class layoutzone_t
{
public:
    enum class zonetype_t
    {
        htile,
        vtile,
        stack
    };

    explicit layoutzone_t(layoutzone_t::zonetype_t type, pos_t pos, dim_t dim)
        : m_type(type), m_pos(pos), m_dim(dim) {}

    void arrange(const user_workspace_t& workspace,
        ::std::pair<::std::deque<client_ptr_t>::const_iterator, ::std::deque<client_ptr_t>::const_iterator>) const;

    inline void set(pos_t pos, dim_t dim) { m_pos = pos; m_dim = dim; }

private:
    zonetype_t m_type;
    pos_t m_pos;
    dim_t m_dim;

}* layoutzone_ptr_t;

typedef class layoutsetting_t
{
public:
    enum class settingtarget_t
    {
        nmaster,
        nstack,
        nclients
    };

    static inline bool greater(::std::size_t lhs, ::std::size_t rhs) { return lhs > rhs; }
    static inline bool less(::std::size_t lhs, ::std::size_t rhs) { return lhs < rhs; }
    static inline bool equal_to(::std::size_t lhs, ::std::size_t rhs) { return lhs == rhs; }

    typedef ::std::function<bool(::std::size_t, ::std::size_t)> opfunc_t;
    static const ::std::size_t minval = ::std::numeric_limits<::std::size_t>::min();

    explicit layoutsetting_t(
        ::std::size_t nmaster = minval, opfunc_t nmaster_op = greater,
        ::std::size_t nstack = minval, opfunc_t nstack_op = greater,
        ::std::size_t nclients = minval, opfunc_t nclients_op = greater)
        : m_nmaster(nmaster), m_nmaster_op(nmaster_op),
          m_nstack(nstack), m_nstack_op(nstack_op),
          m_nclients(nclients), m_nclients_op(nclients_op) {}

    explicit layoutsetting_t(settingtarget_t target, ::std::size_t val, opfunc_t op)
        : layoutsetting_t()
    {
        if (target == settingtarget_t::nmaster) {
            m_nmaster = val;
            m_nmaster_op = op;
        } else if (target == settingtarget_t::nstack) {
            m_nstack = val;
            m_nstack_op = op;
        } else if (target == settingtarget_t::nclients) {
            m_nclients = val;
            m_nclients_op = op;
        }
    }

    bool evaluate(const user_workspace_t&) const;
    void apply(const user_workspace_t&);

    inline void add_masterzone(layoutzone_ptr_t zone) { m_masterzones.push_back(zone); }
    inline void add_stackzone(layoutzone_ptr_t zone) { m_stackzones.push_back(zone); }

private:
    ::std::vector<layoutzone_ptr_t> m_masterzones;
    ::std::vector<layoutzone_ptr_t> m_stackzones;

    ::std::size_t m_nmaster;
    opfunc_t m_nmaster_op;
    ::std::size_t m_nstack;
    opfunc_t m_nstack_op;
    ::std::size_t m_nclients;
    opfunc_t m_nclients_op;

}* layoutsetting_ptr_t;

class layout_t
{
public:
    layout_t() = default;
    layout_t(layouttype_t type)
        : m_type(type) {}

    inline void add_setting(layoutsetting_ptr_t setting) { m_settings.push_back(setting); }

    inline void apply(const user_workspace_t& workspace) const
    {
        for (auto& setting : m_settings)
            if (setting->evaluate(workspace)) {
                setting->apply(workspace);
                return;
            }
    }

private:
    layouttype_t m_type;
    ::std::vector<layoutsetting_ptr_t> m_settings;

};

class layouthandler_t
{
public:
    explicit layouthandler_t(ewmh_t& ewmh)
        : m_ewmh(ewmh)
    {
        add_tile_layout();
        add_stick_layout();
        add_deck_layout();
        add_doubledeck_layout();
        add_grid_layout();
        add_pillar_layout();
        add_column_layout();
        add_monocle_layout();
        add_center_layout();
    }

    const layout_t& get_layout(layouttype_t type) const
    {
        return m_layouts.at(type);
    }

private:
    ewmh_t& m_ewmh;
    ::std::unordered_map<layouttype_t, layout_t> m_layouts;

    void add_tile_layout();
    void add_stick_layout();
    void add_deck_layout();
    void add_doubledeck_layout();
    void add_grid_layout();
    void add_pillar_layout();
    void add_column_layout();
    void add_monocle_layout();
    void add_center_layout();

};

#endif//__KRANEWM_LAYOUT_GUARD__
