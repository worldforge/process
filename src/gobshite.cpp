// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "ClientConnection.h"

#include <iostream>

using Atlas::Message::Object;

bool verbose_flag = false;
bool regress_flag = false;

int main()
{
    ClientConnection c1;

    if (!c1.connect("localhost")) {
        std::cerr << "FATAL: Unable to connect to server"
                  << std::endl << std::flush;
        return 1;
    }

    std::cout << "Blah blah blah!" << std::endl << std::flush;
    Get g(Get::Instantiate());
    for (;;) {
        c1.send(g);
    }
}
