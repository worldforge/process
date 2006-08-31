// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "testCharacter.h"
#include "process_debug.h"

#include <Atlas/Objects/Anonymous.h>

#include <iostream>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Objects::Entity::Anonymous;

void testInvalidCharacterCreate(ClientConnection &c)
{
    verbose( std::cout << "Testing character creation with invalid type"
                           << std::endl << std::flush; );

    Anonymous character;
    character->setObjtype("obj");
    character->setParents(std::list<std::string>(1,"__bad__type__"));
    character->setName("Dwayne");

    Create create;
    create->setFrom(c.getAccountId());
    create->setArgs1(character);
    
    int sno = c.send(create);

    verbose( std::cout << "Waiting for error response to invalid character create"
                           << std::endl << std::flush; );

    if (c.waitForError(sno)) {
        std::cerr << "ERROR: Invalid Character creation did not result in error"
                  << std::endl << std::flush;
    }

    character->setParents(std::list<std::string>());
    
    sno = c.send(create);

    verbose( std::cout << "Waiting for error response to empty character create"
                           << std::endl << std::flush; );

    if (c.waitForError(sno)) {
        std::cerr << "ERROR: Empty Character creation did not result in error"
                  << std::endl << std::flush;
    }
    
    Anonymous non_character;

    create->setArgs1(non_character);

    sno = c.send(create);

    verbose( std::cout << "Waiting for error response to non character create"
                           << std::endl << std::flush; );

    if (c.waitForError(sno)) {
        std::cerr << "ERROR: Non Character creation did not result in error"
                  << std::endl << std::flush;
    }
}

void testCharacterCreate(ClientConnection & connection1,
                         ClientConnection & connection2,
                         ClientConnection & connection3)
{
    verbose( std::cout << "Creating character on primary connection"
                       << std::endl << std::flush; );

    Anonymous character;
    character->setObjtype("obj");
    character->setParents(std::list<std::string>(1,"settler"));
    character->setName("Nivek");

    connection1.createChar(character);
    verbose(std::cout << "created character with ID " << connection1.getCharacterId() <<
            " on connection 1" << std::endl << std::flush; );
    
    if (connection2.isOpen()) {
        character->setName("Cevin");
        connection2.createChar(character);
        verbose(std::cout << "created character with ID " << connection2.getCharacterId() <<
            " on connection 2" << std::endl << std::flush; );
    }

    if (connection3.isOpen()) {
        testInvalidCharacterCreate(connection3);
        
        verbose( std::cout << "Creating character on third connection"
                           << std::endl << std::flush; );

        character->setName("Dwayne");
        connection3.createChar(character);
        verbose(std::cout << "created character with ID " << connection3.getCharacterId() <<
            " on connection 3" << std::endl << std::flush; );
    }

    verbose( std::cout << "Modifying appearance of character on primary connection"
                       << std::endl << std::flush; );

    Anonymous charMod;
    charMod->setId(connection1.getCharacterId());
    charMod->setAttr("height", 4);
    MapType gmap;
    gmap["foo"] = "bar";
    charMod->setAttr("guise", gmap);
    Atlas::Objects::Operation::Set set;
    set->setFrom(connection1.getAccountId());
    set->setArgs1(charMod);

    connection1.send(set);
}

void testCharacterInventory(ClientConnection & connection1)
{
    Atlas::Objects::Operation::Look look;
    Anonymous look_arg;
    look_arg->setId(connection1.getCharacterId());
    look->setFrom(connection1.getCharacterId());
    look->setArgs1(look_arg);

    int sno = connection1.send(look);

    Atlas::Objects::Operation::RootOperation sight = connection1.recv("sight", sno);

    if (!sight.isValid()) {
        std::cerr << "Failed to get sight of character doing IG look" << std::endl;
        return;
    }

    const std::vector<Root> & args =  sight->getArgs();

    if (args.empty()) {
        std::cerr << "Sight of character doing IG look has no args" << std::endl;
        return;
    }

    Atlas::Objects::Entity::RootEntity chr = Atlas::Objects::smart_dynamic_cast<Atlas::Objects::Entity::RootEntity>(args.front());

    if (!chr.isValid()) {
        std::cerr << "Sight of character doing IG look has arg which is not an entity" << std::endl;
        return;
    }

    Element attr_contains;
    if (chr->copyAttr("contains", attr_contains) != 0) {
        std::cerr << "Sight of character doing IG look has no contains attribute" << std::endl;
        return;
    }

    if (!attr_contains.isList()) {
        std::cerr << "Sight of character doing IG look has contains which is not a list" << std::endl;
        return;
    }

    std::vector<Root> combine_args;

    const ListType & chr_inventory = attr_contains.List();
    ListType::const_iterator I = chr_inventory.begin();
    ListType::const_iterator Iend = chr_inventory.end();
    for (; I != Iend; ++I) {
        const Element & iter = *I;
        if (!iter.isString()) {
            std::cerr << "Non string ID in characters contains list" << std::endl << std::flush;
            continue;
        }
        Anonymous combine_item;
        combine_item->setId(iter.String());
        combine_args.push_back(combine_item);
    }

    if (combine_args.empty()) {
        std::cerr << "unable to test stacking cos inventory is empty" << std::endl;
        return;
    }

    Atlas::Objects::Operation::Combine combine;
    combine->setArgs(combine_args);
    combine->setFrom(connection1.getCharacterId());

    connection1.send(combine);
}
