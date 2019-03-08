#ifndef __KRANEWM__FLOATING__GUARD__
#define __KRANEWM__FLOATING__GUARD__

//TODO move, resize
#include "client.hh"
#include "common.hh"
#include "decoration.hh"

enum MoveResizeState {
    MR_INVALID,
    MR_MOVE,
    MR_RESIZE
};

enum Corner {
    NO_CORNER,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT
};

typedef struct moveresize_t
{
    moveresize_t(client_ptr_t _client, MoveResizeState _state, Corner _grabbed)
      : client(_client),
        state(_state),
        grabbed(_grabbed),
        indicator(x_wrapper::create_window(true))
    {
        indicator.set_background_color(MRIND_BG_COLOR);
        indicator.set_border_color(MRIND_BORDER_COLOR);
        indicator.resize({1, 1}).move({SIDEBAR_WIDTH - 3, 1}).map();
    };

    ~moveresize_t()
    {
        //TODO
    }

    client_ptr_t client;
    MoveResizeState state;
    Corner grabbed;
    x_wrapper::window_t indicator;

}* moveresize_ptr_t;

#endif//__KRANEWM__FLOATING__GUARD__
