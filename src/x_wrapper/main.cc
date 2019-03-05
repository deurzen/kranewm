#include "atom.hh"
#include "attributes.hh"
#include "button.hh"
#include "cardinal.hh"
#include "display.hh"
#include "event.hh"
#include "input.hh"
#include "key.hh"
#include "property.hh"
#include "string.hh"
#include "type.hh"
#include "window.hh"

#include <iostream>

using namespace x_wrapper;

int
main(int argc, char **argv)
{
    string_t strrr("NET_WM_NAME");
    atom_t att(55);
    window_t winnn(45);

    property_t propp(winnn);

    ::std::cout << att.type() << ::std::endl;
    ::std::cout << winnn.type() << ::std::endl;
    ::std::cout << strrr.type() << ::std::endl;
    ::std::cout << propp.type() << ::std::endl;

    return 0;
}
