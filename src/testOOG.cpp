// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "testOOG.h"
#include "process_debug.h"

#include <Atlas/Objects/Entity.h>

#include <iostream>

using Atlas::Message::Element;

void testOOG(ClientConnection & connection1,
             ClientConnection & connection2,
             ClientConnection & connection3)
{
    verbose( std::cout << "Sending out-of-game (OOG) look on primary connection"
                       << std::endl << std::flush; );

    Look l;
    l->setFrom(connection1.getAccountId());
    int sno = connection1.send(l);

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
}
