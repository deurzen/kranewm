#ifndef __KRANEWM__KRANEWM__GUARD__
#define __KRANEWM__KRANEWM__GUARD__

#include "x-wrapper/display.hh"
#include "x-wrapper/error.hh"

#include "ewmh.hh"
#include "client-model.hh"
#include "x-model.hh"
#include "x-events.hh"

#include <memory>


class kranewm_t
{
public:
    kranewm_t()
        : m_ewmh(),
          m_clients(),
          m_x(),
          m_events(m_ewmh, m_clients, m_x)
    {}

    void setup();
    void run();

    static ::std::unique_ptr<kranewm_t> init();


private:
    static int otherwmerror(Display*, XErrorEvent*);
    static int (*m_xerrorxlib)(Display*, XErrorEvent*);

    void check_otherwm();

    ewmh_t m_ewmh;
    client_model_t m_clients;
    x_model_t m_x;
    x_events_t m_events;

};

#endif//__KRANEWM__KRANEWM__GUARD__
