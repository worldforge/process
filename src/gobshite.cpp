// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "ClientConnection.h"

#include <Atlas/Objects/loadDefaults.h>

#include <iostream>

bool verbose_flag = false;
bool regress_flag = false;
    
static void usage(const char * progname)
{
    std::cerr << "usage: " << progname << std::endl << std::flush;

}

int main(int argc, char ** argv)
{
    if (argc != 1) {
        usage(argv[0]);
        return 1;
    }

    try {
        Atlas::Objects::loadDefaults("../../protocols/atlas/spec/atlas.xml");
    }
    catch (Atlas::Objects::DefaultLoadingException d) {
        try {
            Atlas::Objects::loadDefaults("atlas.xml");
        }
        catch (Atlas::Objects::DefaultLoadingException d) {
            std::cerr << argv[0] << ": Could not load atlas.xml: "
                      << d.getDescription() << std::endl << std::flush;
            return 1;
        }
    }

    ClientConnection c1;

    if (!c1.connect("localhost")) {
        std::cerr << "FATAL: Unable to connect to server"
                  << std::endl << std::flush;
        return 1;
    }

    std::cout << "Blah blah blah!" << std::endl << std::flush;
    Get g;
    for (;;) {
        c1.send(g);
    }
}
