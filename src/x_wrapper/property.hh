#ifndef __KRANEWM__X_WRAPPER__PROPERTY__GUARD__
#define __KRANEWM__X_WRAPPER__PROPERTY__GUARD__

#include "common.hh"
#include "type.hh"
#include "atom.hh"
#include "window.hh"
#include "string.hh"
#include "cardinal.hh"


namespace x_wrapper
{
    template <class T>
    class property_t : public x_type
    {
    public:
        property_t(const ::std::string& name, T t = T())
            : id(get_atom(name)),
              data(t),
              data_length(t.length()),
              data_type(t.type()),
              type_size(t.size())
        {}

        operator T() { return data; }

        inline int  length() const { return data_length; }
        inline Atom type()   const { return data_type; }
        inline int  size()   const { return type_size; }

        inline Atom get_id()   const { return id; }
        inline T    get_data() const { return data; }

        inline void set_data(void* data_ptr) {
            data = T(data_ptr);
        }

    private:
        Atom id;
        T data;
        int data_length;
        atom_t data_type;
        int type_size;

    };


    extern void remove_property(window_t, const ::std::string&);
    extern bool get_text_property(window_t, atom_t, char*, unsigned int);

    template <typename T>
    bool has_property(Window win, atom_t atom)
    {
        T prop = T();
        int _i;
        unsigned long _ul;
        unsigned char* ucp = nullptr;
        Atom returned_type = None;
        unsigned long n_items_returned = 0;

        return (XGetWindowProperty(g_dpy, win, atom,
            0L, prop.size(), False, prop.type(),
            &returned_type, &_i, &n_items_returned,
            &_ul, &ucp) == Success
            && ucp && XFree(ucp)
            && returned_type == prop.type()
            && n_items_returned > 0);
    }

    template <typename T>
    property_t<T> get_property(Window win, const ::std::string& name)
    {
        int _i;
        unsigned long _ul;
        unsigned char* ucp = nullptr;
        Atom _a = None;
        T data = T();
        property_t<T> prop(name, data);

        if (XGetWindowProperty(g_dpy, win, prop.get_id(),
            0L, prop.size(), False, prop.type(),
            &_a, &_i, &_ul, &_ul, &ucp) == Success && ucp) {
            prop.set_data(ucp);
            XFree(ucp);
        }
        return prop;
    }

    template <typename T>
    void replace_property(window_t win, property_t<T> prop)
    {
        XChangeProperty(g_dpy, win, prop.get_id(), prop.type(), prop.size(),
            PropModeReplace, (unsigned char*) prop.get_data().get_ptr(), prop.length());
    }

    template <typename T>
    void append_property(window_t win, property_t<T> prop)
    {
        XChangeProperty(g_dpy, win, prop.get_id(), prop.type(), prop.size(),
            PropModeAppend, (unsigned char*) prop.get_data().get_ptr(), prop.length());
    }

    template <typename T>
    void unset_property(window_t win, property_t<T> prop)
    {
        XChangeProperty(g_dpy, win, prop.get_id(), prop.type(), prop.size(),
            PropModeReplace, (unsigned char*) 0, 0);
    }
}


#endif//__KRANEWM__X_WRAPPER__PROPERTY__GUARD__
