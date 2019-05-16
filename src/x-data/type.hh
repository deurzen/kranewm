#ifndef __KRANEWM__X_DATA__TYPE__GUARD__
#define __KRANEWM__X_DATA__TYPE__GUARD__

extern "C" {
#include <X11/X.h>
#include <X11/Xatom.h>
}


namespace x_data
{
    class x_type
    {
    protected:
        virtual int  length() { return 0; }
        virtual Atom type()   { return 0; }
        virtual int  size()   { return 0; }
    };
}

#endif//__KRANEWM__X_DATA__TYPE__GUARD__
