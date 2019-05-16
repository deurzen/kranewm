#include "property.hh"

#include <cstring>

using namespace x_data;


void
x_data::remove_property(window_t win, const ::std::string& name)
{
    XDeleteProperty(g_dpy, win.get(), get_atom(name));
}

bool
x_data::get_text_property(window_t win, atom_t atom, char* text, unsigned int size)
{
    char **list = NULL;
    int n;
    XTextProperty name;

    if (!text || size == 0)
        return false;

    text[0] = '\0';

    if (!XGetTextProperty(g_dpy, win, &name, atom) || !name.nitems)
        return false;

    if (name.encoding == XA_STRING)
        ::std::strncpy(text, (char *)name.value, size - 1);
    else if (XmbTextPropertyToTextList(g_dpy, &name, &list, &n)
        >= Success && n > 0 && *list)
    {
        ::std::strncpy(text, *list, size - 1);
        XFreeStringList(list);
    }

    text[size - 1] = '\0';
    XFree(name.value);
    return true;
}


void
x_data::set_text_property(window_t win, property_t<string_list_t> prop)
{
    XTextProperty textprop = prop.get_data().get();
    XSetTextProperty(g_dpy, g_root, &textprop, prop.get_id());
    XFree(textprop.value);
}
