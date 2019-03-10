#ifndef __KRANEWM_STACKING_GUARD__
#define __KRANEWM_STACKING_GUARD__


#include <list>

enum class layer_t
{ // bottom to top
    desktop,
    below,
    normal,
    floating,
    dock,
    above,
    fullscreen
};

// Forward decl
struct client_t;
typedef client_t* client_ptr_t;

class windowstack_t
{

    ::std::list<client_ptr_t> stack;
};


#endif//__KRANEWM_STACKING_GUARD__
