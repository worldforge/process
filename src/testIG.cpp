// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "testIG.h"
#include "process_debug.h"

#include <iostream>

using Atlas::Message::Element;

void testInGameLook(ClientConnection& con)
{
    Look l;
    l->setFrom(con.getCharacterId());
    int serial = con.send(l);
    
    verbose( std::cout << "Waiting for In-game look response on connection "
                       << con.getAccount() << std::endl << std::flush; );

    Element::MapType game_entity_template;
    game_entity_template["id"] = std::string();
    game_entity_template["parents"] = Element::ListType();
    game_entity_template["objtype"] = std::string();
    
    RootOperation anonLookResponse = con.recv("sight", serial);
    if (!anonLookResponse || con.compareArgToTemplate(anonLookResponse, game_entity_template)) {
        std::cerr << "ERROR: In-game anonymous Look failed"
                  << std::endl << std::flush;
    }
}
