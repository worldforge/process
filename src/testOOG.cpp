#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "debug.h"
#include "validator.h"
#include "process_debug.h"
#include "testOOG.h"
#include "atlasUtils.h"

#include <Atlas/Objects/Operation.h>

#include <iostream>

using Atlas::Message::Element;
using Atlas::Objects::Operation::RootOperation;

void TestOOG::setUp()
{
    c.connect("localhost");
}

void TestOOG::tearDown()
{
    c.close();
}

void TestOOG::testCharacterLook()
{
    c.login(c.getAccount(), c.getPassword());
    
    verbose( std::cout << "testCharacterLook: logged in" << std::endl; );
    
    Root account = retrieveAccount(c);
    
    Element::ListType chars = account->getAttr("characters").asList();
    CPPUNIT_ASSERT(!chars.empty());

    // issue a look for character zero
    Element::MapType args;
    args["id"] = chars.front();	// first char ID
    
    Look look;
    look->setFrom(c.getAccount());
    look->setArgsAsList(Element::ListType(1, args));
    
    c.send(look);
    RootOperation recv = c.recv("sight", c.getLastSerialno());
    CPPUNIT_ASSERT_MESSAGE("failed to get a SIGHT response to OOG character LOOK", recv);
    
    // validate what we got back
    Element::MapType entityProto;
    entityProto["id"] = chars.front();
    entityProto["parents"] = Element::ListType(1, "farmer");
    entityProto["name"] = "Nivek";
    validate(getArg(recv), entityProto);
}

void TestOOG::testTakeCharacter()
{
    ClientConnection &con(c);
    
    Root account = retrieveAccount(con);
    Element::ListType chars = account->getAttr("characters").asList();
    CPPUNIT_ASSERT(!chars.empty());

    std::string character(chars.front().asString());
    // use an in game-look to take
    Look take;
    take->setFrom(character);
    Element::MapType args;
    args["id"] = character;	// a world look should be valid here?
    take->setArgsAsList(Element::ListType(1,args));
    
    con.send(take);
    RootOperation recv = con.recv("info", con.getLastSerialno());
    CPPUNIT_ASSERT(recv);
    
    // check info contents
    CPPUNIT_ASSERT(recv->getTo() == con.getAccount());
    
    Element::MapType protoEntity;
    protoEntity["id"] = character;
    protoEntity["name"] = "Nivek";
    protoEntity["parents"] = Element::ListType(1, "farmer");
    validate(getArg(recv), protoEntity);
     
    // ensure the ig-op was generated okay too
    recv = con.recv("sight", con.getLastSerialno());
    CPPUNIT_ASSERT(recv);
    CPPUNIT_ASSERT(recv->getTo() == character);
    validate(getArg(recv), protoEntity);
}

Root TestOOG::retrieveAccount(ClientConnection &con)
{
    CPPUNIT_ASSERT(con.isOpen());

    Look look;
    look->setFrom(con.getAccount());
    
    Element::MapType args;
    args["id"] = con.getAccount();
    
    look->setArgsAsList(Element::ListType(1, args));
    con.send(look);
  
    RootOperation sight = c.recv("sight", con.getLastSerialno());
    CPPUNIT_ASSERT_MESSAGE("failed to recv SIGHT response to OOG account look", sight);
    
    // get the account out
    Root account = getArg(sight);
    
    // validate it a bit
    Element::MapType accountProto;
    accountProto["parents"] = Element::ListType(1, "account");
    accountProto["id"] = con.getAccount();
    validate(account, accountProto);
    
    return account;
}
