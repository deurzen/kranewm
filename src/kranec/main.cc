#include "kranec.hh"


int
main(int argc, char **argv)
{
    auto client = kranec_t::init(argc, argv);

    client->setup();
    client->run();

    client.reset();
    return 0;
}
