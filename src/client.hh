#ifndef __KRANEWM__CLIENT__GUARD__
#define __KRANEWM__CLIENT__GUARD__
//TODO update_child_offset from x_handler
//
//TODO register child etc...
//
//TODO focus() unfocus()

#include "constraints.hh"
#include "rule.hh"

#include <set>


enum clientexpect {
    NO_EFFECT = 0,
    MAP       = 1 << 0,
    WITHDRAW  = 1 << 1,
    ICONIFY   = 1 << 2
};


typedef struct client_t* client_ptr_t;
typedef struct ::std::set<client_ptr_t> client_ptr_set_t;

typedef struct client_t
{
    client_t(x_wrapper::window_t _win, x_wrapper::window_t _frame,
        sizeconstraints_t _sizeconstraints, rule_t& rule)
        : win(_win), frame(_frame), sizeconstraints(_sizeconstraints), expect(NO_EFFECT),
          focused(false), floating(rule.floating), fullscreen(rule.fullscreen),
          shaded(false), iconified(rule.iconify), urgent(false), parent(nullptr)
    {}

    void disown_child(client_ptr_t);

    bool redeem_expect(clientexpect);

    void move(pos_t);
    void resize(dim_t);

    void map();
    void unmap();

    void focus();
    void unfocus(bool);

    void center();

    x_wrapper::window_t win;
    x_wrapper::window_t frame;
    x_wrapper::window_t mr_indicator;
    x_wrapper::window_t float_indicator;
    pos_t               pos;
    pos_t               float_pos;
    dim_t               dim;
    dim_t               float_dim;
    sizeconstraints_t   sizeconstraints;
    clientexpect        expect;
    bool                focused;
    bool                floating;
    bool                fullscreen;
    bool                shaded;
    bool                iconified;
    bool                urgent;
    client_ptr_t        parent;
    client_ptr_set_t    children;

}* client_ptr_t;


extern client_ptr_t create_client(x_wrapper::window_t, rule_t&);

extern void update_offset(client_ptr_t);


#endif//__KRANEWM__CLIENT__GUARD__
