#include "kranewm.hh"


int
main(int argc, char **argv)
{
    auto wm = Kranewm::init();

    wm->setup();
    wm->run();

    return 0;
}
