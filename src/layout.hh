#ifndef __KRANEWM_LAYOUT_GUARD__
#define __KRANEWM_LAYOUT_GUARD__


enum class layout_t : char
{// name         symbol
    toggle      = '_',
    floating    = 'F',
    tile        = 'T',
    stick       = 'S',
    deck        = 'D',
    doubledeck  = '$',
    sdeck       = '@',
    sdoubledeck = '%',
    grid        = '#',
    pillar      = 'P',
    column      = 'C',
    monocle     = 'M',
    center      = '^',
};

// fwd decls
class user_workspace_t;
class ewmh_t;

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
    void layout_sdeck(const user_workspace_t&) const;
    void layout_sdoubledeck(const user_workspace_t&) const;
    void layout_grid(const user_workspace_t&) const;
    void layout_pillar(const user_workspace_t&) const;
    void layout_column(const user_workspace_t&) const;
    void layout_monocle(const user_workspace_t&) const;
    void layout_center(const user_workspace_t&) const;

private:
    ewmh_t& m_ewmh;

};

#endif//__KRANEWM_LAYOUT_GUARD__
