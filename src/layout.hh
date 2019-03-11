#ifndef __KRANEWM_LAYOUT_GUARD__
#define __KRANEWM_LAYOUT_GUARD__


enum class layout_t
{
    toggle,
    floating,
    tile,
    deck,
    doubledeck,
    grid,
    monocle
};

// Forward decl
class user_workspace_t;
class ewmh_t;

class layouthandler_t
{
public:
    explicit layouthandler_t(ewmh_t& ewmh)
        : m_ewmh(ewmh) {}

    void layout_floating(const user_workspace_t&) const;
    void layout_tile(const user_workspace_t&) const;
    void layout_deck(const user_workspace_t&) const;
    void layout_doubledeck(const user_workspace_t&) const;
    void layout_grid(const user_workspace_t&) const;
    void layout_monocle(const user_workspace_t&) const;

private:
    ewmh_t& m_ewmh;

};

#endif//__KRANEWM_LAYOUT_GUARD__
