#ifndef __KRANEWM__X_DATA__EVENT__GUARD__
#define __KRANEWM__X_DATA__EVENT__GUARD__

#include "common.hh"
#include "display.hh"
#include "atom.hh"
#include "window.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xatom.h>
}


namespace x_data
{
    class window_t;
    class event_t
    {
    public:
        event_t() = default;

        event_t(XEvent event)
          : val(event)
        {}

        operator XEvent() const
        {
            return val;
        }

        inline XEvent&
        get()
        {
            return val;
        }

        inline XEvent*
        get_ptr()
        {
            return &val;
        }

        inline int
        type() const
        {
            return val.type;
        }

        event_t& send(long mask, window_t win = None);

    private:
        XEvent val;

    };

    template <class T>
    event_t
    create_event(window_t win, atom_t type, T data)
    {
        XEvent event;
        event.type = ClientMessage;
        event.xclient.window = win;
        event.xclient.message_type = type;
        event.xclient.format = 32;
        event.xclient.data.l[0] = data;
        event.xclient.data.l[1] = CurrentTime;
        return event_t(event);
    }

    extern void enable_substructure_events();
    extern void disable_substructure_events();

    extern void next_event(event_t&);
    extern bool typed_event(event_t&, int);
    extern void last_typed_event(event_t&, int);
    extern void sync(bool);

}

#endif//__KRANEWM__X_DATA__EVENT__GUARD__
