// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001 Alistair Riddoch

#include "ClientConnection.h"
#include "pythonInterface.h"
#include "process_debug.h"

#include <Atlas/Objects/loadDefaults.h>
#include <Atlas/Objects/Entity.h>

#if defined(__GNUC__) && __GNUC__ < 3
#include "sstream.h"
#else
#include <sstream>
#endif

#include <unistd.h>
#include <iostream>

using Atlas::Message::Element;

void testTypeQueries(ClientConnection &c);

void testLogout(ClientConnection &c, const std::string &acc, ClientConnection &watcher);
void testDuplicateLogin(const std::string &account, const std::string &pass);
    
void testInvalidCharacterCreate(ClientConnection &c);    
    
void testInGameLook(ClientConnection& con);    
    
static void usage(const char * progname)
{
    std::cerr << "usage: " << progname << " [-vrsh] [ script ]"
              << std::endl << std::flush;
}

static void help(const char * progname)
{
    usage(progname);
    std::cerr << std::endl;
    std::cerr << "  -v     print verbose descriptions of tests" << std::endl;
    std::cerr << "  -r     supress output that varies between tests"<<std::endl;
    std::cerr << "  -s     supress standard tests" << std::endl;
    std::cerr << "  -h     print this help message" << std::endl;
    std::cerr << std::endl << std::flush;
}

bool verbose_flag = false;
bool regress_flag = false;

int exit_status = 0;

static Element makeAtlasVec(double x, double y, double z)
{
    Element::ListType vec;
    vec.push_back(x);
    vec.push_back(y);
    vec.push_back(z);
    return Element(vec);
}

int main(int argc, char ** argv)
{
    int opt;
    bool script_only_flag = false;

    while ((opt = getopt(argc, argv, "hvrs")) != -1) {
        switch (opt) {
            case 'v':
                verbose_flag = true;
                break;
            case 'r':
                regress_flag = true;
                break;
            case 's':
                script_only_flag = true;
                break;
            case 'h':
            case '?':
                help(argv[0]);
                return 0;
                break;
            default:
                std::cerr << "FATAL: Illegal argument misprocessed by getopt"
                          << std::endl << std::flush;
                usage(argv[0]);
                return 1;
                break;
        }
    };

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

    bool python_script = false;
    std::string script_name;

    if (argc == (optind + 1)) {
        init_python_api();
        python_script = true;
        script_name = argv[optind];
        if (script_only_flag) {
            bool retCode = runScript(script_name);
            if (retCode) {
                return 1;
            }
            return 0;
        }
    } else if (argc != optind) {
        usage(argv[0]);
        return 1;
    }

    if (script_only_flag) {
        return 0;
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

    Get g;
    int sno = connection1.send(RootOperation(g));
    Element::MapType server_template;
    // server_template["id"] = std::string();
    server_template["parents"] = Element::ListType();
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

    if (!connection1.getAccountId().empty()) {
        std::cerr << "ERROR: server description query should not set connection ID" << std::endl;
    }

    std::stringstream ac1, ac2, ac3;

    verbose_only( std::cout << "Creating account with name " << ac1.str()
                            << " on primary connection"
                            << std::endl << std::flush; );
    verbose_regress( std::cout << "Creating test account on primary connection"
                               << std::endl << std::flush; );

    ac1 << getpid() << "testac" << 1;
    sno = connection1.create(ac1.str(), "ptacpw1pc");

    Element::MapType account_template;
    account_template["id"] = std::string();
    account_template["parents"] = Element::ListType();
    account_template["characters"] = Element::ListType();
    account_template["objtype"] = std::string();

    verbose( std::cout << "Waiting for response to account creation on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", account_template, sno)) {
        std::cerr << "FATAL: Unable to create account"
                  << std::endl << std::flush;
        return 1;
    }

        if (connection1.getAccountId().empty()) {
                std::cerr << "FATAL: connection's 1 account ID not set by INFO response" << std::endl;
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

        if (connection2.getAccountId().empty()) {
                std::cerr << "FATAL: connection's 2 account ID not set by INFO response" << std::endl;
                return 1;
        }
        
    Element::MapType appearance_template;
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

        if (connection3.getAccountId().empty()) {
                std::cerr << "FATAL: connection's 3 account ID not set by INFO response" << std::endl;
                return 1;
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

    Look l;
    l->setFrom(connection1.getAccountId());
    sno = connection1.send(l);

    verbose( std::cout << "Waiting for look response on primary connection"
                       << std::endl << std::flush; );

    Element::MapType room_template;
    room_template["id"] = std::string();
    room_template["name"] = std::string();
    room_template["parents"] = Element::ListType();
    room_template["people"] = Element::ListType();
    room_template["rooms"] = Element::ListType();
    room_template["objtype"] = std::string();
    
    RootOperation anonLookResponse = connection1.recv("sight", sno);
    if (!anonLookResponse || connection1.compareArgToTemplate(anonLookResponse, room_template)) {
        std::cerr << "ERROR: Out-of-game Look failed"
                  << std::endl << std::flush;
    }
    
    std::string lobbyId = anonLookResponse->getArgs().front()->getId();
    Root lobby = anonLookResponse->getArgs().front();
    Atlas::Objects::Entity::RootEntity lobent = Atlas::Objects::smart_dynamic_cast<Atlas::Objects::Entity::RootEntity>(lobby);

    verbose( std::cout << "Sending out-of-game (OOG) talk without TO on primary connection"
                       << std::endl << std::flush; );

    Talk t;
    Element::MapType say;
    say["say"] = "Hello";
    say["loc"] = lobbyId;
    t->setFrom(connection1.getAccountId());
    t->setArgsAsList(Element::ListType(1, say));
    sno = connection1.send(t);

    verbose( std::cout << "Waiting for sound response to talk on primary connection"
                       << std::endl << std::flush; );

    Element::MapType talkTemplate;
    talkTemplate["from"] = connection1.getAccountId();
    talkTemplate["args"] = Element::ListType(1, say);

    if (connection1.waitFor("sound", talkTemplate, sno)) {
        std::cerr << "WARNING: Out-of-game Talk did not result in sound"
                  << std::endl << "WARNING: Server may require TO"
                  << std::endl << std::flush;
    }

    if (connection2.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound",talkTemplate, sno)) {
            std::cerr << "WARNING: Out-of-game Talk was not heard by connection 2"
                      << std::endl << "WARNING: Server may require TO"
                      << std::endl << std::flush;
        }
    }

    if (connection3.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on third connection"
                           << std::endl << std::flush; );

        if (connection3.waitFor("sound",talkTemplate, sno)) {
            std::cerr << "WARNING: Out-of-game Talk was not heard by connection 3"
                      << std::endl << "WARNING: Server may require TO"
                      << std::endl << std::flush;
        }
    }

    verbose( std::cout << "Sending out-of-game (OOG) talk on primary connection"
                       << std::endl << std::flush; );

    t->setTo(lobbyId);
    sno = connection1.send(t);

    verbose( std::cout << "Waiting for sound response to talk on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("sound", talkTemplate, sno)) {
        std::cerr << "ERROR: Out-of-game Talk did not result in sound"
                  << std::endl << std::flush;
    }

    if (connection2.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound",talkTemplate, sno)) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 2"
                      << std::endl << std::flush;
        }
    }

    if (connection3.isOpen()) {
        verbose( std::cout << "Waiting for sound response to talk on third connection"
                           << std::endl << std::flush; );

        if (connection3.waitFor("sound",talkTemplate, sno)) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 3"
                      << std::endl << std::flush;
        }
    }


    if (connection2.isOpen()) {
        verbose( std::cout << "Sending out-of-game (OOG) talk on second connection"
                           << std::endl << std::flush; );

        t->setFrom(connection2.getAccountId());
        sno = connection2.send(t);

        verbose( std::cout << "Waiting for sound response to talk on primary connection"
                           << std::endl << std::flush; );

        if (connection1.waitFor("sound",talkTemplate, sno)) {
            std::cerr << "ERROR: Out-of-game Talk was not heard by connection 1"
                      << std::endl << std::flush;
        }

        verbose( std::cout << "Waiting for sound response to talk on second connection"
                           << std::endl << std::flush; );

        if (connection2.waitFor("sound", talkTemplate, sno)) {
            std::cerr << "ERROR: Out-of-game Talk did not result in sound"
                      << std::endl << std::flush;
        }

        if (connection3.isOpen()) {
            verbose( std::cout << "Waiting for sound response to talk on third connection"
                               << std::endl << std::flush; );

            if (connection3.waitFor("sound", talkTemplate, sno)) {
                std::cerr << "ERROR: Out-of-game Talk was not heard by connection 3"
                          << std::endl << std::flush;
            }
        }
    }

    // Talk t();
    // t->SetFrom(connection1.getAccountId());
    // Try out some OOG stuff, like looking, talking and private messages

        // send private chats from 2 -> 1 and 2 -> 3 

    if (connection2.isOpen()) {
        verbose( std::cout << "Sending private out-of-game (OOG) talk on secondary connection"
                           << std::endl << std::flush; );

        t->setTo(connection1.getAccountId());
        t->setFrom(connection2.getAccountId());
        Element::MapType say;
        say["say"] = "Private_2_1";
        t->setArgsAsList(Element::ListType(1, say));

        talkTemplate["args"] = t->getArgsAsList();
        talkTemplate["from"] = connection2.getAccountId();
        talkTemplate["to"] = connection1.getAccountId();
            
        sno = connection2.send(t);

        verbose( std::cout << "Waiting for sound response to private chat on first connection"
                           << std::endl << std::flush; );

        if (connection1.waitFor("sound",talkTemplate, sno)) {
            std::cerr << "ERROR: Out-of-game private chat did not result in sound"
                      << std::endl << std::flush;
        }

        if (connection3.isOpen()) {
            verbose( std::cout << "Sending private out-of-game (OOG) talk on secondary connection"
                               << std::endl << std::flush; );

            t->setTo(connection3.getAccountId());
            say["say"] = "Private_2_3";
            t->setArgsAsList(Element::ListType(1, say));
            sno = connection2.send(t);

            talkTemplate["args"] = t->getArgsAsList();
            talkTemplate["from"] = connection2.getAccountId();
            talkTemplate["to"] = connection3.getAccountId();
        
            verbose( std::cout << "Waiting for sound response to private chat on third connection"
                               << std::endl << std::flush; );

            if (connection3.waitFor("sound", talkTemplate, sno)) {
                std::cerr << "ERROR: Out-of-game private chat did not result in sound"
                          << std::endl << std::flush;
            }

            // FIXME  - verify that each connection receieved the correct string. i.e Private_2_x
        }
    }

    verbose( std::cout << "Creating character on primary connection"
                       << std::endl << std::flush; );

    Element::MapType character;
    character["objtype"] = "object";
    character["parents"] = Element::ListType(1,"settler");
    character["name"] = "Nivek";

    connection1.createChar(character);
    verbose(std::cout << "created character with ID " << connection1.getCharacterId() <<
            " on connection 1" << std::endl << std::flush; );
    
    if (connection2.isOpen()) {
        character["name"] = "Civen";
        connection2.createChar(character);
        verbose(std::cout << "created character with ID " << connection2.getCharacterId() <<
            " on connection 2" << std::endl << std::flush; );
    }

    if (connection3.isOpen()) {
        testInvalidCharacterCreate(connection3);
        
        verbose( std::cout << "Creating character on third connection"
                           << std::endl << std::flush; );

        character["name"] = "Dwayne";
        connection3.createChar(character);
        verbose(std::cout << "created character with ID " << connection3.getCharacterId() <<
            " on connection 3" << std::endl << std::flush; );
    }

    if (python_script) {
        std::set<ClientConnection*> connections;
        connections.insert(&connection1);
        connections.insert(&connection2);
        connections.insert(&connection3);

        bool retCode = runScript(script_name, connections);
        if (retCode) {
            return 1;
        }
    }
    
    // Try out some IG stuff, like creating looking, talking and moving
    testInGameLook(connection1);
    
    return exit_status;
}

void testTypeQueries(ClientConnection &c)
{
    assert(c.isOpen());
    verbose( std::cout << "Requesting root-type" << std::endl; );
    
    Get query;
    Element::MapType arg;
    arg["id"] = "root";
    query->setArgsAsList(Element::ListType(1, arg));
    
    int sno = c.send(query);
    verbose( std::cout << "Waiting for info response to root-type query" << std::endl; );
    
    Element::MapType info;
    info["parents"] = Element::ListType(1, "info");    
    
    if (c.waitFor("info", info, sno)) {
        std::cerr << "ERROR: Type-query for root did not resut in info" << std::endl;
    }
    
    arg["id"] = "game_entity";
    query->setArgsAsList(Element::ListType(1, arg));
    verbose( std::cout << "Requesting info for type game_entity" << std::endl; );
    sno = c.send(query);
    
    verbose( std::cout << "Waiting for info response to game_entity type query" << std::endl; );
    if (c.waitFor("info", info, sno)) {
        std::cerr << "ERROR: Type-query for game_entity did not resut in info" << std::endl;
    }
    
    // try a broken one (unless by some miracle is exists?)
    arg["id"] = "_bad_type_";
    query->setArgsAsList(Element::ListType(1, arg));
    verbose( std::cout << "Requesting info for type _bad_type" << std::endl; );
    sno = c.send(query);
    
    verbose( std::cout << "Waiting for error response to _bad_type_ type query" << std::endl; );
    if (c.waitForError(sno)) {
        std::cerr << "ERROR: Type-query for _bad_type did not resut in error" << std::endl;
    }
}

void testLogout(ClientConnection &c, const std::string &acc, ClientConnection &watcher)
{
    Logout lg;
    lg->setFrom(c.getAccountId());
    verbose( std::cout << "Sending logout for connection 2 (" 
                << c.getAccountId() << std::endl; );
    int sno = c.send(lg);
    
    verbose( std::cout << "Waiting for disappearance of connection 2" << std::endl; );
    Element::MapType disap;
    disap["id"] = Element(acc);  
    if (watcher.waitFor("disappearance", disap)) {
        std::cerr << "ERROR: didn't get a disappearance of account 2" << std::endl;
    }
    
    verbose( std::cout << "Waiting for info(logout) of connection 2" << std::endl; );
    Element::MapType info;
    info["parents"] = Element::ListType(1, "logout");  
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

void testInvalidCharacterCreate(ClientConnection &c)
{
    verbose( std::cout << "Testing character creation with invalid type"
                           << std::endl << std::flush; );

    Element::MapType character;
    character["objtype"] = "object";
    character["parents"] = Element::ListType(1,"__bad__type__");
    character["name"] = "Dwayne";

    Create create;
    create->setFrom(c.getAccountId());
    create->setArgsAsList(Element::ListType(1,character));
    
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
    cr->setFrom(cl.getAccountId());
    
    cr->setTo(lobby);
    
    Element::MapType room;
    room["name"] = "test_room_" + acc;  // make sure it's unique (ish)
    room["loc"] = lobby;
    
    cl.send(cr);
}
*/

/*
void testInGame(ClientConnection &a, ClientConnection &b, ClientConnection &c)
{
    // movement
    Move mv;
    mv->setFrom(a.getAccountId());
    mv->setTo(ac1Char);
    
    Element::MapType move;
    move["id"] = ac1Char;
    move["pos"] = makeAtlasVec(10.0, 0.0, 0.0);
    mv->setArgsAsList(Element::ListType(1, move));

    // velocity movement

    // stop
}
*/

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

void testInGameLook(ClientConnection& con)
{
    Look l;
    l->setFrom(con.getCharacterId());
    int serial = con.send(l);
    
    verbose( std::cout << "Waiting for In-game look response on connection "
                       << con.getAccount() << std::endl << std::flush; );

    Element::MapType game_entity_template;
    game_entity_template["id"] = std::string();
    game_entity_template["name"] = std::string();
    game_entity_template["parents"] = Element::ListType();
    game_entity_template["objtype"] = std::string();
    
    RootOperation anonLookResponse = con.recv("sight", serial);
    if (!anonLookResponse || con.compareArgToTemplate(anonLookResponse, game_entity_template)) {
        std::cerr << "ERROR: In-game anonymous Look failed"
                  << std::endl << std::flush;
    }
}
