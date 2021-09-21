#include <iostream>

#include "../winsys/xdata/xconnection.hh"
#include "defaults.hh"
#include "model.hh"

int
main(int, char **)
{
    XConnection conn{ WM_NAME };
    Model model(conn);
    model.run();
    return 0;
}
