// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

#include "ClientConnection.h"
#include "pythonInterface.h"
#include "tests.h"
#include "testOOG.h"
#include "testCharacter.h"
#include "testIG.h"
#include "process_debug.h"

#include <Atlas/Objects/loadDefaults.h>
#include <Atlas/Objects/Entity.h>

#if defined(__GNUC__) && __GNUC__ < 3
#include "sstream.h"
#else
#include <sstream>
#endif

#include <iostream>

using Atlas::Message::Element;

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

    {
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
    int sno = connection1.create(ac1.str(), "ptacpw1pc");

    {
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
    }

    testDuplicateLogin(ac1.str(), "ptacpw1pc");
    
    testLogout(connection2, ac2.str(), connection1);
    
    testTypeQueries(connection1);
    
    testOOG(connection1, connection2, connection3);

    testCharacterCreate(connection1, connection2, connection3);

    // Try out some IG stuff, like creating looking, talking and moving
    testInGameLook(connection1);
    
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
    
    return exit_status;
}
