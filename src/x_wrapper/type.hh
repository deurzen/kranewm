#ifndef __KRANEWM__X_WRAPPER__TYPE__GUARD__
#define __KRANEWM__X_WRAPPER__TYPE__GUARD__

extern "C" {
#include <X11/X.h>
}


namespace x_wrapper
{

    class x_type
    {
    protected:
        virtual Atom type();
        virtual int size();
    };

}

#endif//__KRANEWM__X_WRAPPER__TYPE__GUARD__
