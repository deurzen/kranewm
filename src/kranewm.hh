#ifndef __KRANEWM__KRANEWM__GUARD__
#define __KRANEWM__KRANEWM__GUARD__

#include "x_wrapper/display.hh"
#include "x_wrapper/error.hh"

#include "ewmh.hh"
#include "client_model.hh"
#include "x_events.hh"

#include <memory>


class Kranewm
{
public:
    Kranewm()
        : m_events(m_ewmh, m_clients)
    {}

    void setup();
    void run();

    static ::std::unique_ptr<Kranewm> init();


private:
    static int otherwmerror(Display*, XErrorEvent*);
    static int (*m_xerrorxlib)(Display*, XErrorEvent*);

    void check_otherwm();

    ewmh m_ewmh;
    client_model m_clients;
    x_events m_events;

};

#endif//__KRANEWM__KRANEWM__GUARD__
