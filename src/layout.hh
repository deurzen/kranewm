#ifndef __KRANEWM_LAYOUT_GUARD__
#define __KRANEWM_LAYOUT_GUARD__

#include <functional>
#include <limits>
#include <vector>

// fwd decls
class user_workspace_t;
class ewmh_t;


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

class layoutzone_t
{
    enum class zonetype_t
    {
        htile,
        vtile,
        stack
    };

public:
    void arrange(const user_workspace_t& workspace) const;

private:
    zonetype_t m_type;

};

class layoutsetting_t
{
    static inline bool greater(::std::size_t lhs, ::std::size_t rhs) { return lhs > rhs; }
    static inline bool less(::std::size_t lhs, ::std::size_t rhs) { return lhs < rhs; }
    static inline bool equal_to(::std::size_t lhs, ::std::size_t rhs) { return lhs == rhs; }

    typedef ::std::function<bool(::std::size_t, ::std::size_t)> opfunc_t;
    static const ::std::size_t minval = ::std::numeric_limits<::std::size_t>::min();

public:
    explicit layoutsetting_t(
        ::std::size_t nmaster = minval, opfunc_t nmaster_op = greater,
        ::std::size_t nstack = minval, opfunc_t nstack_op = greater,
        ::std::size_t nclients = minval, opfunc_t nclients_op = greater)
        : m_nmaster(nmaster), m_nmaster_op(nmaster_op),
          m_nstack(nstack), m_nstack_op(nstack_op),
          m_nclients(nclients), m_nclients_op(nclients_op) {}

    bool evaluate(const user_workspace_t&) const;

    inline void add_zone(const layoutzone_t&& zone)
    {
        m_zones.push_back(zone);
    }

    inline void apply(const user_workspace_t& workspace) const
    {
        for (auto& zone : m_zones)
            zone.arrange(workspace);
    }

private:
    ::std::vector<layoutzone_t> m_zones;

    const ::std::size_t m_nmaster;
    const opfunc_t m_nmaster_op;
    const ::std::size_t m_nstack;
    const opfunc_t m_nstack_op;
    const ::std::size_t m_nclients;
    const opfunc_t m_nclients_op;

};

class layout_t
{
public:
    inline void apply(const user_workspace_t& workspace)
    {
        for (auto& setting : m_settings)
            if (setting.evaluate(workspace)) {
                setting.apply(workspace);
                return;
            }
    }

private:
    ::std::vector<layoutsetting_t> m_settings;

};

class layouthandler_t
{
public:
    explicit layouthandler_t(ewmh_t& ewmh)
        : m_ewmh(ewmh) {}

    void layout_floating(const user_workspace_t&) const;
    void layout_tile(const user_workspace_t&) const;
    void layout_stick(const user_workspace_t&) const;
    void layout_deck(const user_workspace_t&) const;
    void layout_doubledeck(const user_workspace_t&) const;
    void layout_grid(const user_workspace_t&) const;
    void layout_pillar(const user_workspace_t&) const;
    void layout_column(const user_workspace_t&) const;
    void layout_monocle(const user_workspace_t&) const;
    void layout_center(const user_workspace_t&) const;

private:
    ewmh_t& m_ewmh;

};

#endif//__KRANEWM_LAYOUT_GUARD__
