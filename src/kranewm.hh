#ifndef __KRANEWM__KRANEWM__GUARD__
#define __KRANEWM__KRANEWM__GUARD__

#include "x-wrapper/display.hh"
#include "x-wrapper/error.hh"

#include "ewmh.hh"
#include "client-model.hh"
#include "x-model.hh"
#include "x-events.hh"

#include <memory>


class kranewm
{
public:
    kranewm()
        : m_ewmh(),
          m_clients(),
          m_x(),
          m_events(m_ewmh, m_clients, m_x)
    {}

    void setup();
    void run();

    static ::std::unique_ptr<kranewm> init();


private:
    static int otherwmerror(Display*, XErrorEvent*);
    static int (*m_xerrorxlib)(Display*, XErrorEvent*);

    void check_otherwm();

    ewmh m_ewmh;
    client_model m_clients;
    x_model m_x;
    x_events m_events;

};

#endif//__KRANEWM__KRANEWM__GUARD__
