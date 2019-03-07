#ifndef __KRANEWM__X_WRAPPER__REQUEST__GUARD__
#define __KRANEWM__X_WRAPPER__REQUEST__GUARD__

#include "event.hh"

namespace x_wrapper
{
    extern void propagate_configure_request(event_t, unsigned);
    extern void propagate_circulate_request(event_t);
}


#endif//__KRANEWM__X_WRAPPER__REQUEST__GUARD__
