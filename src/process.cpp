// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "ClientConnection.h"
#include "pythonInterface.h"
#include "process.h"

#include "sstream.h"

#include <unistd.h>

using Atlas::Message::Object;

void usage(const char * progname)
{
    std::cerr << "usage: " << progname << " [-vr] [ script ]"
              << std::endl << std::flush;

}

bool verbose_flag = false;
bool regress_flag = false;

int exit_status = 0;

int main(int argc, char ** argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "vr")) != -1) {
        switch (opt) {
            case 'v':
                verbose_flag = true;
                break;
            case 'r':
                regress_flag = true;
                break;
            case '?':
                return 1;
                break;
            default:
                std::cerr << "FATAL: Illegal argument misprocessed by getopt"
                          << std::endl << std::flush;
                return 1;
                break;
        }
    };
    if (argc == (optind + 1)) {
        init_python_api();
        if (runScript(argv[optind])) {
            return exit_status;
        } else {
            exit_status = 1;
            return exit_status;
        }
    } else if (argc != optind) {
        usage(argv[0]);
        return 1;
    }

    ClientConnection connection1, connection2, connection3;

    verbose( std::cout << "Making primary connection to server on localhost"
                       << std::endl << std::flush; );

    if (!connection1.connect("localhost")) {
        std::cerr << "FATAL: Unable to connect to server"
                  << std::endl << std::flush;
        return 1;
    }

    verbose( std::cout << "Making second connection to server on localhost"
                       << std::endl << std::flush; );

    if (!connection2.connect("localhost")) {
        std::cerr << "FATAL: Unable to make second connection to server"
                  << std::endl << std::flush;
        return 1;
    }

    verbose( std::cout << "Making third connection to server on localhost"
                       << std::endl << std::flush; );

    if (!connection3.connect("localhost")) {
        std::cerr << "FATAL: Unable to make third connection to server"
                  << std::endl << std::flush;
        return 1;
    }

    verbose( std::cout << "Sending Get on primary connection to get server description"
                       << std::endl << std::flush; );

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

    verbose( std::cout << "Waiting for server description on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", server_template)) {
        std::cerr << "ERROR: Unable to query server information"
                  << std::endl << std::flush;
    }

    std::stringstream ac1, ac2, ac3;

    verbose_only( std::cout << "Creating account of name " << ac1.str()
                            << " on primary connection"
                            << std::endl << std::flush; );
    verbose_regress( std::cout << "Creating test account on primary connection"
                               << std::endl << std::flush; );

    ac1 << getpid() << "testac" << 1;
    connection1.create(ac1.str(), "ptacpw1pc");

    Object::MapType account_template;
    account_template["id"] = std::string();
    account_template["parents"] = Object::ListType();
    account_template["characters"] = Object::ListType();
    account_template["objtype"] = std::string();

    verbose( std::cout << "Waiting for response to account creation on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", account_template)) {
        std::cerr << "FATAL: Unable to create account"
                  << std::endl << std::flush;
        return 1;
    }

    ac2 << getpid() << "testac" << 2;

    verbose_only( std::cout << "Creating account of name " << ac2.str()
                            << " on second connection"
                            << std::endl << std::flush; );
    verbose_regress( std::cout << "Creating test account on second connection"
                               << std::endl << std::flush; );

    connection2.create(ac2.str(), "ptacpw2pc");

    verbose( std::cout << "Waiting for response to account creation on second connection"
                       << std::endl << std::flush; );

    if (connection2.waitFor("info", account_template)) {
        std::cerr << "ERROR: Unable to create second account"
                  << std::endl << std::flush;
        connection2.close();
    }

    Object::MapType appearance_template;
    appearance_template["id"] = std::string();
    appearance_template["loc"] = std::string();

    verbose( std::cout << "Waiting for appearance of account 2 on connection 1"
                       << std::endl << std::flush; );

    if (connection1.waitFor("appearance", appearance_template)) {
        std::cerr << "ERROR: First connection did not get appearance of second account creation"
                  << std::endl << std::flush;
    }

    ac3 << getpid() << "testac" << 3;

    verbose_only( std::cout << "Creating account of name " << ac3.str()
                            << " on third connection"
                            << std::endl << std::flush; );
    verbose_regress( std::cout << "Creating test account on third connection"
                               << std::endl << std::flush; );

    connection3.create(ac3.str(), "ptacpw3pc");

    verbose( std::cout << "Waiting for response to account creation on third connection"
                       << std::endl << std::flush; );

    if (connection3.waitFor("info", account_template)) {
        std::cerr << "ERROR: Unable to create third account"
                  << std::endl << std::flush;
        connection3.close();
    }

    verbose( std::cout << "Waiting for appearance of account 3 on connections 1 & 2"
                       << std::endl << std::flush; );

    if (connection1.waitFor("appearance", appearance_template)) {
        std::cerr << "ERROR: First connection did not get appearance of third account creation"
                  << std::endl << std::flush;
    }

    if (connection2.waitFor("appearance", appearance_template)) {
        std::cerr << "ERROR: Second connection did not get appearance of third account creation"
                  << std::endl << std::flush;
    }

    verbose( std::cout << "Closing primary connection to server"
                       << std::endl << std::flush; );

    connection1.close();

    verbose( std::cout << "Waiting for disappearance of account 1 on connections 2 & 3"
                       << std::endl << std::flush; );

    if (connection2.waitFor("disappearance", appearance_template)) {
        std::cerr << "ERROR: Second connection did not get disappearance of account 1 logout"
                  << std::endl << std::flush;
    }

    if (connection3.waitFor("disappearance", appearance_template)) {
        std::cerr << "ERROR: Third connection did not get appearance of account 1 logout"
                  << std::endl << std::flush;
    }

    verbose( std::cout << "Re-opening primary connection to server"
                       << std::endl << std::flush; );

    if (!connection1.connect("localhost")) {
        std::cerr << "ERROR: Unable to re-connect to server"
                  << std::endl << std::flush;
        return 1;
    }

    verbose_only( std::cout << "Logging in to account name " << ac1.str()
                            << " on primary connection"
                            << std::endl << std::flush; );
    verbose_regress( std::cout << "Logging test account on third connection"
                               << std::endl << std::flush; );

    connection1.login(ac1.str(), "ptacpw1pc");

    verbose( std::cout << "Waiting for login response on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", account_template)) {
        std::cerr << "FATAL: Unable to login to pre-created account"
                  << std::endl << std::flush;
        return 1;
    }

    verbose( std::cout << "Waiting for appearance of account 1 on connections 2 & 3"
                       << std::endl << std::flush; );

    if (connection2.waitFor("appearance", appearance_template)) {
        std::cerr << "ERROR: Second connection did not get appearance of account 1 login"
                  << std::endl << std::flush;
    }

    if (connection3.waitFor("appearance", appearance_template)) {
        std::cerr << "ERROR: Third connection did not get appearance of account 1 login"
                  << std::endl << std::flush;
    }

    verbose( std::cout << "Sending out-of-game (OOG) look on primary connection"
                       << std::endl << std::flush; );

    Look l(Look::Instantiate());
    l.SetFrom(ac1.str());
    connection1.send(l);

    verbose( std::cout << "Waiting for look response on primary connection"
                       << std::endl << std::flush; );

    Object::MapType room_template;
    room_template["id"] = std::string();
    room_template["name"] = std::string();
    room_template["parents"] = Object::ListType();
    room_template["people"] = Object::ListType();
    room_template["rooms"] = Object::ListType();
    room_template["objtype"] = std::string();
    if (connection1.waitFor("sight", room_template)) {
        std::cerr << "ERROR: Out-of-game Look failed"
                  << std::endl << std::flush;
    }

    verbose( std::cout << "Sending out-of-game (OOG) talk without TO on primary connection"
                       << std::endl << std::flush; );

    Talk t(Talk::Instantiate());
    Object::MapType say;
    say["say"] = "Hello";
    say["loc"] = "lobby";
    t.SetFrom(ac1.str());
    t.SetArgs(Object::ListType(1, say));
    connection1.send(t);

    verbose( std::cout << "Waiting for sound response to talk on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("sound", t.AsObject().AsMap())) {
        std::cerr << "WARNING: Out-of-game Talk did not result in sound"
                  << std::endl << "WARNING: Server may require TO"
                  << std::endl << std::flush;
    }

    if (connection2.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound", t.AsObject().AsMap())) {
            std::cerr << "WARNING: Out-of-game Talk was not heard by connection 2"
                      << std::endl << "WARNING: Server may require TO"
                      << std::endl << std::flush;
        }
    }

    if (connection3.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on third connection"
                           << std::endl << std::flush; );

        if (connection3.waitFor("sound", t.AsObject().AsMap())) {
            std::cerr << "WARNING: Out-of-game Talk was not heard by connection 3"
                      << std::endl << "WARNING: Server may require TO"
                      << std::endl << std::flush;
        }
    }

    verbose( std::cout << "Sending out-of-game (OOG) talk on primary connection"
                       << std::endl << std::flush; );

    t.SetTo("lobby");
    connection1.send(t);
    
    verbose( std::cout << "Waiting for sound response to talk on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("sound", t.AsObject().AsMap())) {
        std::cerr << "ERROR: Out-of-game Talk did not result in sound"
                  << std::endl << std::flush;
    }

    if (connection2.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound", t.AsObject().AsMap())) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 2"
                      << std::endl << std::flush;
        }
    }

    if (connection3.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on third connection"
                           << std::endl << std::flush; );

        if (connection3.waitFor("sound", t.AsObject().AsMap())) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 3"
                      << std::endl << std::flush;
        }
    }

    
    if (connection2.isOpen()) {
        verbose( std::cout << "Sending out-of-game (OOG) talk on second connection"
                           << std::endl << std::flush; );

        t.SetFrom(ac2.str());
        connection2.send(t);

        verbose( std::cout << "Waiting for sound response to talk on primary connection"
                           << std::endl << std::flush; );

        if (connection1.waitFor("sound", t.AsObject().AsMap())) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 1"
                      << std::endl << std::flush;
        }

        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound", t.AsObject().AsMap())) {
            std::cerr << "ERROR: Out-of-game Talk did not result in sound"
                      << std::endl << std::flush;
        }

        if (connection3.isOpen()) {
            verbose( std::cout << "Waiting for sound response to talk on third connection"
                               << std::endl << std::flush; );

            if (connection3.waitFor("sound", t.AsObject().AsMap())) {
                std::cerr << "ERROR: Out-of-game Talk was not heard by connection 3"
                          << std::endl << std::flush;
            }
        }
    }

    // Talk t(Talk::Instantiate());
    // t->SetFrom(ac1.str());
    // Try out some OOG stuff, like looking, talking and private messages

	// send private chats from 2 -> 1 and 2 -> 3 
	
	if (connection2.isOpen()) {
		t.SetTo(ac1.str());
		t.SetFrom(ac2.str());
		Object::MapType say;
    	say["say"] = "Private_2_1";
    	t.SetArgs(Object::ListType(1, say));
		connection2.send(t);
		
		t.SetTo(ac3.str());
    	say["say"] = "Private_2_3";
    	t.SetArgs(Object::ListType(1, say));
		connection2.send(t);
		
		verbose( std::cout << "Waiting for sound response to private chat on first connection"
                           << std::endl << std::flush; );
		
		if (connection1.waitFor("sound", t.AsObject().AsMap())) {
			std::cerr << "ERROR: Out-of-game private chat did not result in sound"
                      << std::endl << std::flush;
		}
		
		verbose( std::cout << "Waiting for sound response to private chat on third connection"
                           << std::endl << std::flush; );
		
		if (connection3.waitFor("sound", t.AsObject().AsMap())) {
			std::cerr << "ERROR: Out-of-game private chat did not result in sound"
                      << std::endl << std::flush;
		}
		
		// FIXME  - verify that each connection receieved the correct string. i.e Private_2_x
	}
	
    // Try out some IG stuff, like creating looking, talking and moving
    return exit_status;
}
