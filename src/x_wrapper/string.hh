#ifndef __KRANEWM__X_WRAPPER__STRING__GUARD__
#define __KRANEWM__X_WRAPPER__STRING__GUARD__

#include "common.hh"
#include "type.hh"
#include "atom.hh"

#include <string>


namespace x_wrapper
{
    class string_t : public x_type
    {
        using wrapped_type = char;

    public:
        string_t() = default;

        string_t(const char* c_str)
            : val(c_str)
        {}

        explicit string_t(void* raw_data)
            : val((wrapped_type*)raw_data)
        {}

        operator ::std::string() const { return val; }
        operator bool() const { return !val.empty(); }

        inline int  length() const { return val.size(); }
        inline Atom type()   const { return get_atom("UTF8_STRING"); }
        inline int  size()   const { return 8; }

        inline ::std::string get() const { return val; }
        inline const char* get_ptr() const { return val.c_str(); }

    private:
        ::std::string val;

    };
}

#endif//__KRANEWM__X_WRAPPER__STRING__GUARD__
