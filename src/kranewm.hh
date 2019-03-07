#ifndef __KRANEWM__KRANEWM__GUARD__
#define __KRANEWM__KRANEWM__GUARD__

#include "x_wrapper/display.hh"
#include "x_wrapper/error.hh"

#include "ewmh.hh"

#include <memory>


class Kranewm
{
public:
    Kranewm() = default;

    void setup();
    void run();

    static ::std::unique_ptr<Kranewm> init();


private:
    static int otherwmerror(Display*, XErrorEvent*);
    static int (*m_xerrorxlib)(Display*, XErrorEvent*);

    void check_otherwm();

    EWMH m_ewmh;

};


#endif//__KRANEWM__KRANEWM__GUARD__
