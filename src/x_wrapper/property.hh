#ifndef __KRANEWM__X_WRAPPER__PROPERTY__GUARD__
#define __KRANEWM__X_WRAPPER__PROPERTY__GUARD__

#include "type.hh"
#include "atom.hh"
#include "window.hh"
#include "string.hh"
#include "cardinal.hh"


namespace x_wrapper
{
    template <class T>
    class property_t : protected x_type
    {
    public:
        explicit property_t(T t)
            : data(t),
              atom(t.type()),
              size(t.size()),
              raw_data(t)
        {}


    private:
        T data;
        atom_t atom;
        unsigned char* raw_data;
        int size;

    };

}


#endif//__KRANEWM__X_WRAPPER__PROPERTY__GUARD__
