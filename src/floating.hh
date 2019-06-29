#ifndef __KRANEWM__FLOATING__GUARD__
#define __KRANEWM__FLOATING__GUARD__

#include "common.hh"
#include "decoration.hh"

#include "x-data/window.hh"

// fwd decls
typedef class client_t* client_ptr_t;


enum class moveresizestate_t {
    invalid,
    move,
    resize
};

enum class corner_t {
    no_corner,
    top_left,
    top_right,
    bottom_left,
    bottom_right
};

typedef struct moveresize_t
{
    moveresize_t(client_ptr_t _client, moveresizestate_t _state, corner_t _grabbed_at)
      : client(_client),
        state(_state),
        grabbed_at(_grabbed_at),
        indicator(x_data::create_window(true))
    {
        indicator.set_background_color(MRIND_BG_COLOR);
        indicator.set_border_width(0);
        indicator.resize({1, 1}).move({-1, -1}).map();
    };

    ~moveresize_t()
    {
        indicator.unmap().destroy();
    }

    void process_move_increment(pos_t, dim_t, pos_t);
    void process_resize_increment(pos_t, dim_t, pos_t);

    client_ptr_t client;
    moveresizestate_t state;
    corner_t grabbed_at;
    x_data::window_t indicator;

}* moveresize_ptr_t;

#endif//__KRANEWM__FLOATING__GUARD__
