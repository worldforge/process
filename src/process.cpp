// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "ClientConnection.h"
#include "pythonInterface.h"

#include "sstream.h"

using Atlas::Message::Object;

int main(int argc, char ** argv)
{
    init_python_api();
    runScript();
#if 0
    ClientConnection connection1, connection2, connection3;

    if (!connection1.connect("localhost")) {
        std::cerr << "FATAL: Unable to connect to server"
                  << std::endl << std::flush;
        return 1;
    }

    if (!connection2.connect("localhost")) {
        std::cerr << "FATAL: Unable to make second connection to server"
                  << std::endl << std::flush;
        return 1;
    }

    if (!connection3.connect("localhost")) {
        std::cerr << "FATAL: Unable to make third connection to server"
                  << std::endl << std::flush;
        return 1;
    }

    Get g(Get::Instantiate());
    connection1.send(g);
    Object::MapType server_template;
    // server_template["id"] = std::string();
    server_template["parents"] = Object::ListType();
    server_template["name"] = std::string();
    server_template["server"] = std::string();
    server_template["clients"] = 0;
    server_template["uptime"] = 1.1;
    server_template["objtype"] = std::string();
    if (connection1.waitFor("info", server_template)) {
        std::cerr << "ERROR: Unable to query server information"
                  << std::endl << std::flush;
    }

    

    std::stringstream ac1, ac2, ac3;

    ac1 << getpid() << "testac" << 1;
    connection1.create(ac1.str(), "ptacpw1pc");
    Object::MapType account_template;
    account_template["id"] = std::string();
    account_template["parents"] = Object::ListType();
    account_template["characters"] = Object::ListType();
    account_template["objtype"] = std::string();
    if (connection1.waitFor("info", account_template)) {
        std::cerr << "FATAL: Unable to create account"
                  << std::endl << std::flush;
        return 1;
    }

    ac2 << getpid() << "testac" << 2;
    connection2.create(ac2.str(), "ptacpw2pc");
    if (connection2.waitFor("info", account_template)) {
        std::cerr << "ERROR: Unable to create second account"
                  << std::endl << std::flush;
        connection2.close();
    }

    ac3 << getpid() << "testac" << 3;
    connection3.create(ac3.str(), "ptacpw3pc");
    if (connection3.waitFor("info", account_template)) {
        std::cerr << "ERROR: Unable to create third account"
                  << std::endl << std::flush;
        connection3.close();
    }

    connection1.close();

    if (!connection1.connect("localhost")) {
        std::cerr << "ERROR: Unable to re-connect to server"
                  << std::endl << std::flush;
        return 1;
    }

    connection1.login(ac1.str(), "ptacpw1pc");
    if (connection1.waitFor("info", account_template)) {
        std::cerr << "FATAL: Unable to login to pre-created account"
                  << std::endl << std::flush;
        return 1;
    }

    Look l(Look::Instantiate());
    l.SetFrom(ac1.str());
    Object::MapType room_template;
    room_template["id"] = std::string();
    room_template["name"] = std::string();
    room_template["parents"] = Object::ListType();
    room_template["people"] = Object::ListType();
    room_template["rooms"] = Object::ListType();
    room_template["objtype"] = std::string();
    connection1.send(l);
    if (connection1.waitFor("sight", room_template)) {
        std::cerr << "ERROR: Out-of-game Look failed"
                  << std::endl << std::flush;
    }
    
    // Talk t(Talk::Instantiate());
    // t->SetFrom(ac1.str());
    // Try out some OOG stuff, like looking, talking and private messages

    // Try out some IG stuff, like creating looking, talking and moving
    return 0;
#endif
}
