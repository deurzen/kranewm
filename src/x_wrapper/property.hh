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
              data_type(t.type()),
              data_size(t.size())
        {}

        inline int size() const { return data_size; }
        inline Atom type() const { return data_type; }

    private:
        T data;
        atom_t data_type;
        unsigned char* raw_data;
        int data_size;

    };

}


#endif//__KRANEWM__X_WRAPPER__PROPERTY__GUARD__
