// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "ClientConnection.h"
#include "pythonInterface.h"
#include "process_debug.h"

#if defined(__GNUC__) && __GNUC__ < 3
#include "sstream.h"
#else
#include <sstream>
#endif

#include <unistd.h>
#include <iostream>

using Atlas::Message::Object;

void testTypeQueries(ClientConnection &c);

void testLogout(ClientConnection &c, const std::string &acc, ClientConnection &watcher);
void testDuplicateLogin(const std::string &account, const std::string &pass);
    
void testInvalidCharacterCreate(ClientConnection &c, const std::string &acc);    
    
void usage(const char * progname)
{
    std::cerr << "usage: " << progname << " [-vr] [ script ]"
              << std::endl << std::flush;

}

Object makeAtlasVec(double x, double y, double z);

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
    int sno = connection1.send(g);
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

    if (connection1.waitFor("info", server_template, sno)) {
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
    sno = connection1.create(ac1.str(), "ptacpw1pc");

    Object::MapType account_template;
    account_template["id"] = std::string();
    account_template["parents"] = Object::ListType();
    account_template["characters"] = Object::ListType();
    account_template["objtype"] = std::string();

    verbose( std::cout << "Waiting for response to account creation on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", account_template, sno)) {
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

    sno = connection2.create(ac2.str(), "ptacpw2pc");

    verbose( std::cout << "Waiting for response to account creation on second connection"
                       << std::endl << std::flush; );

    if (connection2.waitFor("info", account_template, sno)) {
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

    sno = connection3.create(ac3.str(), "ptacpw3pc");

    verbose( std::cout << "Waiting for response to account creation on third connection"
                       << std::endl << std::flush; );

    if (connection3.waitFor("info", account_template, sno)) {
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

    sno = connection1.login(ac1.str(), "ptacpw1pc");

    verbose( std::cout << "Waiting for login response on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", account_template, sno)) {
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

    testDuplicateLogin(ac1.str(), "ptacpw1pc");
    
    testLogout(connection2, ac2.str(), connection1);
    
    testTypeQueries(connection1);
    
    verbose( std::cout << "Sending out-of-game (OOG) look on primary connection"
                       << std::endl << std::flush; );

    Look l(Look::Instantiate());
    l.SetFrom(ac1.str());
    sno = connection1.send(l);

    verbose( std::cout << "Waiting for look response on primary connection"
                       << std::endl << std::flush; );

    Object::MapType room_template;
    room_template["id"] = std::string();
    room_template["name"] = std::string();
    room_template["parents"] = Object::ListType();
    room_template["people"] = Object::ListType();
    room_template["rooms"] = Object::ListType();
    room_template["objtype"] = std::string();
    if (connection1.waitFor("sight", room_template, sno)) {
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
    sno = connection1.send(t);

    verbose( std::cout << "Waiting for sound response to talk on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("sound", t.AsObject().AsMap(), sno)) {
        std::cerr << "WARNING: Out-of-game Talk did not result in sound"
                  << std::endl << "WARNING: Server may require TO"
                  << std::endl << std::flush;
    }

    if (connection2.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound", t.AsObject().AsMap(), sno)) {
            std::cerr << "WARNING: Out-of-game Talk was not heard by connection 2"
                      << std::endl << "WARNING: Server may require TO"
                      << std::endl << std::flush;
        }
    }

    if (connection3.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on third connection"
                           << std::endl << std::flush; );

        if (connection3.waitFor("sound", t.AsObject().AsMap(), sno)) {
            std::cerr << "WARNING: Out-of-game Talk was not heard by connection 3"
                      << std::endl << "WARNING: Server may require TO"
                      << std::endl << std::flush;
        }
    }

    verbose( std::cout << "Sending out-of-game (OOG) talk on primary connection"
                       << std::endl << std::flush; );

    t.SetTo("lobby");
    sno = connection1.send(t);

    verbose( std::cout << "Waiting for sound response to talk on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("sound", t.AsObject().AsMap(), sno)) {
        std::cerr << "ERROR: Out-of-game Talk did not result in sound"
                  << std::endl << std::flush;
    }

    if (connection2.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound", t.AsObject().AsMap(), sno)) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 2"
                      << std::endl << std::flush;
        }
    }

    if (connection3.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on third connection"
                           << std::endl << std::flush; );

        if (connection3.waitFor("sound", t.AsObject().AsMap(), sno)) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 3"
                      << std::endl << std::flush;
        }
    }


    if (connection2.isOpen()) {
        verbose( std::cout << "Sending out-of-game (OOG) talk on second connection"
                           << std::endl << std::flush; );

        t.SetFrom(ac2.str());
        sno = connection2.send(t);

        verbose( std::cout << "Waiting for sound response to talk on primary connection"
                           << std::endl << std::flush; );

        if (connection1.waitFor("sound", t.AsObject().AsMap(), sno)) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 1"
                      << std::endl << std::flush;
        }

        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound", t.AsObject().AsMap(), sno)) {
            std::cerr << "ERROR: Out-of-game Talk did not result in sound"
                      << std::endl << std::flush;
        }

        if (connection3.isOpen()) {
            verbose( std::cout << "Waiting for sound response to talk on third connection"
                               << std::endl << std::flush; );

            if (connection3.waitFor("sound", t.AsObject().AsMap(), sno)) {
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
        verbose( std::cout << "Sending private out-of-game (OOG) talk on secondary connection"
                           << std::endl << std::flush; );

        t.SetTo(ac1.str());
        t.SetFrom(ac2.str());
        Object::MapType say;
        say["say"] = "Private_2_1";
        t.SetArgs(Object::ListType(1, say));

        sno = connection2.send(t);

        verbose( std::cout << "Waiting for sound response to private chat on first connection"
                           << std::endl << std::flush; );

        if (connection1.waitFor("sound", t.AsObject().AsMap(), sno)) {
            std::cerr << "ERROR: Out-of-game private chat did not result in sound"
                      << std::endl << std::flush;
        }

        if (connection3.isOpen()) {
            verbose( std::cout << "Sending private out-of-game (OOG) talk on secondary connection"
                               << std::endl << std::flush; );

            t.SetTo(ac3.str());
            say["say"] = "Private_2_3";
            t.SetArgs(Object::ListType(1, say));
            sno = connection2.send(t);

            verbose( std::cout << "Waiting for sound response to private chat on third connection"
                               << std::endl << std::flush; );

            if (connection3.waitFor("sound", t.AsObject().AsMap(), sno)) {
                std::cerr << "ERROR: Out-of-game private chat did not result in sound"
                          << std::endl << std::flush;
            }

            // FIXME  - verify that each connection receieved the correct string. i.e Private_2_x
        }
    }

    verbose( std::cout << "Creating character on primary connection"
                       << std::endl << std::flush; );

    Object::MapType character;
    character["objtype"] = "object";
    character["parents"] = Object::ListType(1,"farmer");
    character["name"] = "Nivek";

    Create create = Create::Instantiate();
    create.SetFrom(ac1.str());
    create.SetArgs(Object::ListType(1,character));

    sno = connection1.send(create);

    verbose( std::cout << "Waiting for info response to character creation on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", character, sno)) {
        std::cerr << "ERROR: Character creation did not result in info"
                  << std::endl << std::flush;
    }
    
    if (connection2.isOpen()) {
        verbose( std::cout << "Creating character on second connection"
                           << std::endl << std::flush; );

        character["name"] = "Cevin";
        create.SetFrom(ac2.str());
        create.SetArgs(Object::ListType(1,character));

        sno = connection2.send(create);

        verbose( std::cout << "Waiting for info response to character creation on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("info", character, sno)) {
            std::cerr << "ERROR: Character creation did not result in info"
                      << std::endl << std::flush;
        }
    }

    if (connection3.isOpen()) {
	testInvalidCharacterCreate(connection3, ac3.str());
	
        verbose( std::cout << "Creating character on third connection"
                           << std::endl << std::flush; );

        character["name"] = "Dwayne";
        create.SetFrom(ac3.str());
        create.SetArgs(Object::ListType(1,character));

        sno = connection3.send(create);

        verbose( std::cout << "Waiting for info response to character creation on third connection"
                           << std::endl << std::flush; );

        if (connection3.waitFor("info", character, sno)) {
            std::cerr << "ERROR: Character creation did not result in info"
                      << std::endl << std::flush;
        }
    }
    
    // Try out some IG stuff, like creating looking, talking and moving
    return exit_status;
}

void testTypeQueries(ClientConnection &c)
{
    assert(c.isOpen());
    verbose( std::cout << "Requesting root-type" << std::endl; );
    
    Get query = Get::Instantiate();
    Object::MapType arg;
    arg["id"] = "root";
    query.SetArgs(Object::ListType(1, arg));
    
    int sno = c.send(query);
    verbose( std::cout << "Waiting for info response to root-type query" << std::endl; );
    
    Object::MapType info;
    info["parents"] = Object::ListType(1, "info");    
    
    if (c.waitFor("info", info, sno)) {
	std::cerr << "ERROR: Type-query for root did not resut in info" << std::endl;
    }
    
    arg["id"] = "game_entity";
    query.SetArgs(Object::ListType(1, arg));
    verbose( std::cout << "Requesting info for type game_entity" << std::endl; );
    sno = c.send(query);
    
    verbose( std::cout << "Waiting for info response to game_entity type query" << std::endl; );
    if (c.waitFor("info", info, sno)) {
	std::cerr << "ERROR: Type-query for game_entity did not resut in info" << std::endl;
    }
    
    // try a broken one (unless by some miracle is exists?)
    arg["id"] = "_bad_type_";
    query.SetArgs(Object::ListType(1, arg));
    verbose( std::cout << "Requesting info for type _bad_type" << std::endl; );
    sno = c.send(query);
    
    verbose( std::cout << "Waiting for error response to _bad_type_ type query" << std::endl; );
    if (c.waitForError(sno)) {
	std::cerr << "ERROR: Type-query for _bad_type did not resut in error" << std::endl;
    }
}

void testLogout(ClientConnection &c, const std::string &acc, ClientConnection &watcher)
{
    Logout lg = Logout::Instantiate();
    lg.SetFrom(acc);
    verbose( std::cout << "Sending logut for connection 2" << std::endl; );
    int sno = c.send(lg);
    
    verbose( std::cout << "Waiting for disappearance of connection 2" << std::endl; );
    Object::MapType disap;
    disap["id"] = Object(acc);  
    if (watcher.waitFor("disappearance", disap)) {
	std::cerr << "ERROR: didn't get a disappearance of account 2" << std::endl;
    }
    
    verbose( std::cout << "Waiting for info(logout) of connection 2" << std::endl; );
    Object::MapType info;
    info["parents"] = Object::ListType(1, "logout");  
    if (c.waitFor("info", info, sno)) {
	std::cerr << "NOTE: LOGOUT did not produce an INFO response; this is okay could be fixed" << std::endl;
    }
    
    c.close();
    
    if (!c.connect("localhost")) {
	std::cerr << "ERROR: Unable to re-connect to server"
                  << std::endl << std::flush;
        return;
    }
    
    sno = c.login(acc, "ptacpw2pc");
    
    verbose( std::cout << "Waiting for info(player) of connection 2" << std::endl; );
    if (c.waitFor("info", info, sno)) {
	std::cerr << "ERROR: login did not produce an INFO response" << std::endl;
    }
    
}

void testInvalidCharacterCreate(ClientConnection &c, const std::string &acc)
{
    verbose( std::cout << "Testing character creation with invalid type"
                           << std::endl << std::flush; );

    Object::MapType character;
    character["objtype"] = "object";
    character["parents"] = Object::ListType(1,"__bad__type__");
    character["name"] = "Dwayne";

    Create create = Create::Instantiate();
    create.SetFrom(acc);
    create.SetArgs(Object::ListType(1,character));
    
    int sno = c.send(create);

    verbose( std::cout << "Waiting for error response to invalid character create"
                           << std::endl << std::flush; );

    if (c.waitForError(sno)) {
	std::cerr << "ERROR: Invalid Character creation did not result in error"
	          << std::endl << std::flush;
    }
}



/*

void testRooms(ClientConnection &cl, const std::string &acc)
{
    Create cr;
    cr.SetFrom(acc);
    
    cr.SetTo(lobby);
    
    Object::MapType room;
    room["name"] = "test_room_" + acc;	// make sure it's unique (ish)
    room["loc"] = lobby;
    
    cl.send(cr);
}
*/

/*
void testInGame(ClientConnection &a, ClientConnection &b, ClientConnection &c)
{
    // movement
    Move mv = Move::Instantiate();
    mv.SetFrom(ac1Char);
    mv.SetTo(ac1Char);
    
    Object::MapType move;
    move["id"] = ac1Char;
    move["pos"] = makeAtlasVec(10.0, 0.0, 0.0);
    mv.SetArgs(Object::ListType(1, move));

    // velocity movement

    // stop
}
*/

Object makeAtlasVec(double x, double y, double z)
{
    Object::ListType vec;
    vec.push_back(x);
    vec.push_back(y);
    vec.push_back(z);
    return Object(vec);
}

void testDuplicateLogin(const std::string &account, const std::string &pass)
{
    ClientConnection dup;
    if (!dup.connect("localhost")) {
	std::cerr << "ERROR: Unable to connect to server"
                  << std::endl << std::flush;
        return;
    }
    
    dup.login(account, pass);
    
    verbose( std::cout << "Waiting for ERROR on duplication login to account " 
	<< account << std::endl; );
    
    if (dup.waitForError()) {
	std::cerr << "ERROR: duplicate login did not produce an ERROR response" << std::endl;
    }
    
    dup.close();
}
