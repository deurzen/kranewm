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
    window_t win = create_window(true);
    replace_property<cardinal_t>(win, {"TEST", 99});
    ::std::cout << has_property<cardinal_t>(win, "TEST") << ::std::endl;
    property_t ok = get_property<cardinal_t>(win, "TEST");
    cardinal_t kk = ok.get_data();
    ::std::cout << kk.get() << ::std::endl;

    disable_substructure_events();
    enable_substructure_events();
    enable_substructure_events();

    return 0;
}
