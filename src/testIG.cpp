// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "testIG.h"
#include "process_debug.h"

#include <Atlas/Objects/RootEntity.h>
#include <Atlas/Objects/RootOperation.h>
#include <iostream>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;
using Atlas::Objects::Operation::RootOperation;
using Atlas::Objects::Entity::RootEntity;

static void lookAtEntity(ClientConnection& con, const std::string & eid,
                                                const std::string & loc)
{
    Look l;
    l->setFrom(con.getCharacterId());
    MapType lookEnt;
    lookEnt["id"] = eid;
    l->setArgsAsList(ListType(1, lookEnt));
    int serial = con.send(l);
    
    verbose( std::cout << "Waiting for In-game look response on connection "
                       << con.getAccount() << std::endl << std::flush; );

    MapType game_entity_template;
    game_entity_template["id"] = std::string();
    game_entity_template["parents"] = ListType();
    game_entity_template["objtype"] = std::string();
    
    RootOperation sight = con.recv("sight", serial);
    if (!sight.isValid() || con.compareArgToTemplate(sight, game_entity_template)) {
        std::cerr << "ERROR: In-game Look failed" << std::endl << std::flush;
        return;
    }

    const std::vector<Atlas::Objects::Root> & args = sight->getArgs();
    if (args.empty()) {
        std::cerr << "ERROR: SIGHT has no arguments set" << std::endl << std::flush;
    } else {
        RootEntity ent = Atlas::Objects::smart_dynamic_cast<RootEntity>(args[0]);
        if (ent->getLoc() != loc)
             std::cerr << "ERROR: entity has incorrect LOC value"
                       << ent->getLoc() << ":" << loc
                       << std::endl << std::flush;
             
        if (ent->getId() != eid)
             std::cerr << "ERROR: entity has incorrect ID value" << std::endl << std::flush;
    }
}

static void lookAtChildren(ClientConnection& con, const RootEntity & ent)
{
    const std::list<std::string> & children = ent->getContains();
    verbose(std::cout << "entity has " << children.size() << " children"
                      << std::endl << std::flush;);
        
    if (children.size() > 0)
    {
        std::vector<std::string> cv(children.begin(), children.end());
        unsigned int childA = random() % cv.size(), 
            childB = random() % cv.size();
            
        lookAtEntity(con, cv[childA], ent->getId()); 
        lookAtEntity(con, cv[childB], ent->getId());   
    }
}

static void touchEntity(ClientConnection& con, const std::string& eid)
{
    Touch t;
    t->setFrom(con.getCharacterId());
    MapType touchEnt;
    touchEnt["id"] = eid;
    t->setArgsAsList(ListType(1, touchEnt));

    con.send(t);
}

static void testTouchChildren(ClientConnection& con, const RootEntity& ent)
{
     const std::list<std::string> & children = ent->getContains();
    if (children.empty())
        return;
        
    std::vector<std::string> cv(children.begin(), children.end());
    unsigned int childA = random() % cv.size(), 
        childB = random() % cv.size();
        
    touchEntity(con, cv[childA]); 
    touchEntity(con, cv[childB]);   
}

void testInGameLook(ClientConnection& con)
{
// look at the top-level visible
    Look l;
    l->setFrom(con.getCharacterId());
    int serial = con.send(l);
    
    verbose( std::cout << "Waiting for In-game look response on connection "
                       << con.getAccount() << std::endl << std::flush; );

    MapType game_entity_template;
    game_entity_template["id"] = std::string();
    game_entity_template["parents"] = ListType();
    game_entity_template["objtype"] = std::string();
    
    RootOperation anonLookResponse = con.recv("sight", serial);
    if (!anonLookResponse.isValid() || con.compareArgToTemplate(anonLookResponse, game_entity_template)) {
        std::cerr << "ERROR: In-game anonymous Look failed"
                  << std::endl << std::flush;
    }
    
// pick some random children and look at them too 
    std::vector<Atlas::Objects::Root> args = anonLookResponse->getArgs();
    if (args.empty()) {
        std::cerr << "ERROR: SIGHT of world has no arguments set" << std::endl << std::flush;
    } else {
        RootEntity ent = Atlas::Objects::smart_static_cast<RootEntity>(args[0]);
        
        if (ent->getLoc() != "") {
             std::cerr << "ERROR: TLVE has non-null LOC value" << std::endl << std::flush;
        }
        
        lookAtChildren(con, ent);
        
        testTouchChildren(con, ent);
    }
    
// let's look at ourselves ....    
    MapType lookEnt;
    lookEnt["id"] = con.getCharacterId();
    l->setArgsAsList(ListType(1, lookEnt));
    serial = con.send(l);
    
    verbose( std::cout << "Waiting for self IG look response on connection "
                       << con.getAccount() << std::endl << std::flush; );
    
    RootOperation selfLookResponse = con.recv("sight", serial);
    if (!selfLookResponse.isValid() || con.compareArgToTemplate(selfLookResponse, game_entity_template)) {
        std::cerr << "ERROR: In-game self Look failed" << std::endl << std::flush;
    }

// pick some random children and look at them too 
    args = selfLookResponse->getArgs();
    if (!args.empty()) {
        RootEntity ent = Atlas::Objects::smart_static_cast<RootEntity>(args[0]);

        if (ent->getId() != con.getCharacterId()) {
            std::cerr << "ERROR: self look entity has incorrect ID value "
                      << ent->getId() << ":" << con.getCharacterId()
                      << std::endl << std::flush;
        }
        
        lookAtChildren(con, ent);
    }
}
