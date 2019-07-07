#ifndef __KRANEWM__CLIENT__GUARD__
#define __KRANEWM__CLIENT__GUARD__

#include "constraints.hh"
#include "rule.hh"

#include <set>


enum class clientaction_t : int
{
    remove = 0,
    add    = 1,
    toggle = 2
};

enum class clientexpect_t : int
{
    noeffect = 0,
    map      = 1 << 0,
    withdraw = 1 << 1
};

enum class snapedge_t
{
    noedge,
    north,
    east,
    south,
    west
};


typedef struct client_t* client_ptr_t;
typedef struct ::std::set<client_ptr_t> client_ptr_set_t;

typedef struct client_t
{
    client_t() = default;
    client_t(x_data::window_t _win, x_data::window_t _frame,
        sizeconstraints_t _sizeconstraints, rule_t& rule)
        : name(_win.get_class()),
          win(_win),
          frame(_frame),
          sizeconstraints(_sizeconstraints),
          expect(clientexpect_t::noeffect),
          focused(false),
          floating(rule.floating),
          fullscreen(rule.fullscreen),
          urgent(false),
          sticky(false),
          parent(nullptr) {}

    void disown_child(client_ptr_t);
    bool consume_expect(clientexpect_t);
    void must_expect(clientexpect_t);
    client_t& set_float(clientaction_t);

    client_t& move(pos_t, bool = false);
    client_t& resize(dim_t, bool = false);

    client_t& map();
    client_t& unmap();
    client_t& map_children();
    client_t& unmap_children();

    client_t& center();
    client_t& snap(snapedge_t);

    ::std::string       name;
    x_data::window_t    win;
    x_data::window_t    frame;
    pos_t               pos;
    pos_t               float_pos;
    dim_t               dim;
    dim_t               float_dim;
    sizeconstraints_t   sizeconstraints;
    clientexpect_t      expect;
    bool                focused;
    bool                floating;
    bool                fullscreen;
    bool                urgent;
    bool                sticky;
    client_ptr_t        parent;
    client_ptr_set_t    children;

}* client_ptr_t;


extern client_ptr_t create_client(x_data::window_t, rule_t&);
extern void update_offset(client_ptr_t);

#endif//__KRANEWM__CLIENT__GUARD__
