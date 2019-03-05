#ifndef __KRANEWM__X_WRAPPER__STRING__GUARD__
#define __KRANEWM__X_WRAPPER__STRING__GUARD__

#include "type.hh"
#include "atom.hh"

#include <string>


namespace x_wrapper
{
    class string_t : protected x_type
    {
    public:
        string_t() = default;

        explicit string_t(const char* c_str)
            : val(c_str)
        {}

        operator ::std::string() const { return val; }
        operator bool() const { return !val.empty(); }

        inline int size() const { return val.size(); }
        inline Atom type() const { return get_atom("UTF8_STRING"); }

    private:
        ::std::string val;

    };
}

#endif//__KRANEWM__X_WRAPPER__STRING__GUARD__
