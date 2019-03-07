#ifndef __KRANEWM__X_WRAPPER__STRING__GUARD__
#define __KRANEWM__X_WRAPPER__STRING__GUARD__

#include "common.hh"
#include "type.hh"
#include "atom.hh"

extern "C" {
#include <X11/extensions/Xrandr.h>
}

#include <algorithm>
#include <cstring>
#include <vector>
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

        string_t(const ::std::string& str)
            : val(str)
        {}

        explicit string_t(void* raw_data)
            : val((wrapped_type*)raw_data)
        {}

        operator ::std::string() const { return val; }
        operator bool() const { return !val.empty(); }

        inline int  length() const { return val.size(); }
        inline Atom type()   const { return get_atom("UTF8_STRING"); }
        inline int  size()   const { return 8; }

        inline ::std::string get()     const { return val; }
        inline const char*   get_ptr() const { return val.c_str(); }

    private:
        ::std::string val;

    };

    class string_list_t : public x_type
    {
        using wrapped_type = char*;

    public:
        string_list_t() = default;

        string_list_t(char** c_str_list, size_t list_length)
            : val(c_str_list, c_str_list + list_length), len(list_length)
        {}

        string_list_t(::std::vector<::std::string> str_list)
            : len(str_list.size())
        {
            ::std::transform(str_list.begin(), str_list.end(),
                ::std::back_inserter(val),
                [](::std::string& text) {
                    char* mutable_text = new char[text.size() + 1];
                    ::std::strcpy(mutable_text, text.c_str());
                    return mutable_text;
                });
        }

        explicit string_list_t(void* raw_data, unsigned long data_len)
            : len(data_len)
        {
            char** c_str_list = (char**)raw_data;
            val.clear();
            val = ::std::vector<char*>(&c_str_list[0], &c_str_list[data_len]);
        }

        operator char**() { return &val[0]; }
        operator bool() const { return val[0] != nullptr; }

        inline int  length() const { return len; }
        inline Atom type()   const { return get_atom("UTF8_STRING"); }
        inline int  size()   const { return 8; }

        inline XTextProperty get()
        {
            XTextProperty prop;
            Xutf8TextListToTextProperty(g_dpy, &val[0], len,
                XUTF8StringStyle, &prop);
            return prop;
        }

        inline char** get_ptr() { return &val[0]; }

    private:
        ::std::vector<char*> val;
        size_t len;

    };
}

#endif//__KRANEWM__X_WRAPPER__STRING__GUARD__
