#ifndef __KRANEWM__X_WRAPPER__CARDINAL__GUARD__
#define __KRANEWM__X_WRAPPER__CARDINAL__GUARD__

#include "type.hh"

extern "C" {
#include <X11/Xmd.h>
#include <X11/Xatom.h>
}

#include <vector>


namespace x_wrapper
{

    class cardinal_t : protected x_type
    {
    public:
        cardinal_t() = default;

        operator CARD32() const { return val; }
        operator bool() const { return val != 0; }

        inline int size() const { return 1; }
        inline Atom type() const { return XA_CARDINAL; }

    private:
        CARD32 val;

    };


    class cardinal_list_t : protected x_type
    {
    public:
        cardinal_list_t() = default;

        operator ::std::vector<CARD32>() const { return vals; }
        operator bool() const { return !vals.empty(); }

        inline int size() const { return vals.size(); }
        inline Atom type() const { return XA_CARDINAL; }

    private:
        ::std::vector<CARD32> vals;

    };

}

#endif//__KRANEWM__X_WRAPPER__CARDINAL__GUARD__
