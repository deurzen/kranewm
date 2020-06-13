#ifndef __kranec__KRANEC__KRANEC__GUARD__
#define __kranec__KRANEC__KRANEC__GUARD__

#include "ipc.hh"
#include "ewmh.hh"

#include "../x-data/property.hh"
#include "../x-data/display.hh"
#include "../x-data/error.hh"

#include <memory>

typedef class kranec_t* kranec_ptr_t;
static kranec_ptr_t g_instance;

class kranec_t final
{
public:
    kranec_t(int argc, char** argv)
      : m_running(true),
        m_ipc(--argc, ++argv),
        m_ewmh()
    {
        // global instance to enable stateful signal handling
        g_instance = this;
    }

    ~kranec_t()
    {
        x_data::g_dpy.close();
    }

    void setup();
    void run();
    void exit();

    static ::std::unique_ptr<kranec_t> init(int, char**);

private:
    static int (*m_xerrorxlib)(Display*, XErrorEvent*);

    static void wait_children(int);
    static void handle_signal(int);

    void check_kranewm();
    void init_signals();

    bool m_running;

    ipc_t m_ipc;
    ewmh_t m_ewmh;
};


#endif//__kranec__KRANEC__KRANEC__GUARD__
