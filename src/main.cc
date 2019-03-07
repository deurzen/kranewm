#include "kranewm.hh"


int
main(int argc, char **argv)
{
    auto wm = kranewm::init();

    wm->setup();
    wm->run();

    return 0;
}
