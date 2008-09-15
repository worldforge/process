// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "testLogin.h"
#include "process_debug.h"

#if defined(__GNUC__) && __GNUC__ < 3
#include "sstream.h"
#else
#include <sstream>
#endif

#include <iostream>

#include <cstdlib>

using Atlas::Message::Element;
using Atlas::Message::MapType;
using Atlas::Message::ListType;

void testLogin(ClientConnection & connection1,
               ClientConnection & connection2,
               ClientConnection & connection3)
{
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
    int sno = connection1.create(ac1.str(), "ptacpw1pc");

    MapType account_template;
    account_template["id"] = std::string();
    account_template["parents"] = ListType();
    account_template["characters"] = ListType();
    account_template["objtype"] = std::string();

    verbose( std::cout << "Waiting for response to account creation on primary connection"
                       << std::endl << std::flush; );

    if (connection1.waitFor("info", account_template, sno)) {
        std::cerr << "FATAL: Unable to create account"
                  << std::endl << std::flush;
        exit(1);
    }

    if (connection1.getAccountId().empty()) {
            std::cerr << "FATAL: connection's 1 account ID not set by INFO response" << std::endl;
            exit(1);
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
            exit(1);
    }
        
    MapType appearance_template;
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
            exit(1);
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
        exit(1);
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
        exit(1);
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
}
