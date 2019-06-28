#include "kranewm.hh"


int
main(int argc, char **argv)
{
    auto wm = kranewm_t::init();

    wm->setup();
    wm->run();

    wm.reset();
    return 0;
}
