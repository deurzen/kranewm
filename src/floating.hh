#ifndef __KRANEWM__FLOATING__GUARD__
#define __KRANEWM__FLOATING__GUARD__

//TODO move, resize
#include "client.hh"

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
        grabbed(_grabbed)
    {};

    client_ptr_t client;
    MoveResizeState state;
    Corner grabbed;

}* moveresize_ptr_t;

#endif//__KRANEWM__FLOATING__GUARD__
