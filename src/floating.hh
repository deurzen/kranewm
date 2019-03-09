#ifndef __KRANEWM__FLOATING__GUARD__
#define __KRANEWM__FLOATING__GUARD__

//TODO move, resize
#include "client.hh"
#include "common.hh"
#include "decoration.hh"

enum moveresizestate {
    MR_INVALID,
    MR_MOVE,
    MR_RESIZE
};

enum corner {
    NO_CORNER,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

typedef struct moveresize_t
{
    moveresize_t(client_ptr_t _client, moveresizestate _state, corner _grabbed_at)
      : client(_client),
        state(_state),
        grabbed_at(_grabbed_at),
        indicator(x_wrapper::create_window(true))
    {
        indicator.set_background_color(MRIND_BG_COLOR);
        indicator.set_border_color(MRIND_BORDER_COLOR);
        indicator.resize({1, 1}).move({SIDEBAR_WIDTH - 3, 1}).map();
    };

    ~moveresize_t()
    {
        indicator.unmap().destroy();
    }

    void process_move_increment(pos_t, dim_t, pos_t);
    void process_resize_increment(pos_t, dim_t, pos_t);

    client_ptr_t client;
    moveresizestate state;
    corner grabbed_at;
    x_wrapper::window_t indicator;

}* moveresize_ptr_t;

#endif//__KRANEWM__FLOATING__GUARD__
