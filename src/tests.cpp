// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "tests.h"
#include "process_debug.h"

#include <iostream>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;

#if 0
static Element makeAtlasVec(double x, double y, double z)
{
    Element::ListType vec;
    vec.push_back(x);
    vec.push_back(y);
    vec.push_back(z);
    return Element(vec);
}
#endif

void testTypeQueries(ClientConnection &c)
{
    assert(c.isOpen());
    verbose( std::cout << "Requesting root-type" << std::endl; );
    
    Get query;
    MapType arg;
    arg["id"] = "root";
    query->setArgsAsList(ListType(1, arg));
    
    int sno = c.send(query);
    verbose( std::cout << "Waiting for info response to root-type query" << std::endl; );
    
    MapType info;
    info["parents"] = ListType(1, "info");    
    
    if (c.waitFor("info", info, sno)) {
        std::cerr << "ERROR: Type-query for root did not resut in info" << std::endl;
    }
    
    arg["id"] = "game_entity";
    query->setArgsAsList(ListType(1, arg));
    verbose( std::cout << "Requesting info for type game_entity" << std::endl; );
    sno = c.send(query);
    
    verbose( std::cout << "Waiting for info response to game_entity type query" << std::endl; );
    if (c.waitFor("info", info, sno)) {
        std::cerr << "ERROR: Type-query for game_entity did not resut in info" << std::endl;
    }
    
    // try a broken one (unless by some miracle is exists?)
    arg["id"] = "_bad_type_";
    query->setArgsAsList(ListType(1, arg));
    verbose( std::cout << "Requesting info for type _bad_type" << std::endl; );
    sno = c.send(query);
    
    verbose( std::cout << "Waiting for error response to _bad_type_ type query" << std::endl; );
    if (c.waitForError(sno)) {
        std::cerr << "ERROR: Type-query for _bad_type did not resut in error" << std::endl;
    }
}

void testLogout(ClientConnection &c, ClientConnection &watcher)
{
    std::string acc = c.getAccount();
    Logout lg;
    lg->setFrom(c.getAccountId());
    verbose( std::cout << "Sending logout for connection 2 (" 
                << c.getAccountId() << std::endl; );
    int sno = c.send(lg);
    
    verbose( std::cout << "Waiting for disappearance of connection 2" << std::endl; );
    MapType disap;
    disap["id"] = Element(acc);  
    if (watcher.waitFor("disappearance", disap)) {
        std::cerr << "ERROR: didn't get a disappearance of account 2" << std::endl;
    }
    
    verbose( std::cout << "Waiting for info(logout) of connection 2" << std::endl; );
    MapType info;
    info["parents"] = ListType(1, "logout");  
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
    
    int refNo = dup.login(account, pass);
    
    verbose( std::cout << "Waiting for ERROR on duplication login to account " 
        << account << std::endl; );
    
    if (dup.waitForError(refNo)) {
        std::cerr << "ERROR: duplicate login did not produce an ERROR response" << std::endl;
    }
    
    dup.close();
}
