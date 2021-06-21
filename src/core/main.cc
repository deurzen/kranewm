#include <iostream>

#include "../winsys/xdata/xconnection.hh"
#include "model.hh"

int
main(int, char **)
{
    XConnection conn = {};
    Model model(conn);
    model.run();
    return 0;
}
