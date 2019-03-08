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


enum ClientExpect {
    NO_EFFECT = 0,
    MAP       = 1 << 0,
    WITHDRAW  = 1 << 1,
    ICONIFY   = 1 << 2
};


typedef struct client_t* client_ptr_t;
typedef struct ::std::set<client_ptr_t> client_ptr_set_t;

typedef struct client_t
{
    client_t(x_wrapper::window_t _win, x_wrapper::window_t _frame, Rule& rule)
        : win(_win), frame(_frame), expect(NO_EFFECT), focused(false),
          floating(rule.floating), fullscreen(rule.fullscreen), shaded(false),
          iconified(rule.iconify), urgent(false), parent(nullptr)
    {}

    void disown_child(client_ptr_t);

    bool redeem_expect(ClientExpect);

    void move(Pos);
    void resize(Size);

    void map();
    void unmap();

    void focus();
    void unfocus(bool);

    void center();

    x_wrapper::window_t win;
    x_wrapper::window_t frame;
    x_wrapper::window_t mr_indicator;
    x_wrapper::window_t float_indicator;
    Pos                 pos;
    Pos                 float_pos;
    Size                size;
    Size                float_size;
    sizeconstraints_t   sizeconstraints;
    ClientExpect        expect;
    bool                focused;
    bool                floating;
    bool                fullscreen;
    bool                shaded;
    bool                iconified;
    bool                urgent;
    client_ptr_t        parent;
    client_ptr_set_t    children;

}* client_ptr_t;


extern client_ptr_t create_client(x_wrapper::window_t, Rule&);

extern void update_offset(client_ptr_t);


#endif//__KRANEWM__CLIENT__GUARD__
