#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "debug.h"
#include "validator.h"
#include "process_debug.h"
#include "testOOG.h"
#include "atlasUtils.h"

#include <Atlas/Objects/Operation/Look.h>

#include <iostream>

using namespace Atlas::Message;
using namespace Atlas::Objects::Operation;

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
    
    Object account = retrieveAccount(c);
    
    Object::ListType chars = account.AsMap()["characters"].AsList();
    CPPUNIT_ASSERT(!chars.empty());

    // issue a look for character zero
    Object::MapType args;
    args["id"] = chars.front();	// first char ID
    
    Look look;
    look.SetFrom(c.getAccount());
    look.SetArgs(Object::ListType(1, args));
    
    c.send(look);
    RootOperation *recv = c.recv("sight", c.getLastSerialno());
    CPPUNIT_ASSERT_MESSAGE("failed to get a SIGHT response to OOG character LOOK", recv);
    
    // validate what we got back
    Object::MapType entityProto;
    entityProto["id"] = chars.front();
    entityProto["parents"] = Object::ListType(1, "farmer");
    entityProto["name"] = "Nivek";
    validate(getArg(recv), entityProto);
}

void TestOOG::testTakeCharacter()
{
    ClientConnection &con(c);
    
    Object account = retrieveAccount(con);
    Object::ListType chars = account.AsMap()["characters"].AsList();
    CPPUNIT_ASSERT(!chars.empty());

    std::string character(chars.front().AsString());
    // use an in game-look to take
    Look take;
    take.SetFrom(character);
    Object::MapType args;
    args["id"] = character;	// a world look should be valid here?
    take.SetArgs(Object::ListType(1,args));
    
    con.send(take);
    RootOperation *recv = con.recv("info", con.getLastSerialno());
    CPPUNIT_ASSERT(recv);
    
    // check info contents
    CPPUNIT_ASSERT(recv->GetTo() == con.getAccount());
    
    Object::MapType protoEntity;
    protoEntity["id"] = character;
    protoEntity["name"] = "Nivek";
    protoEntity["parents"] = Object::ListType(1, "farmer");
    validate(getArg(recv), protoEntity);
     
    // ensure the ig-op was generated okay too
    recv = con.recv("sight", con.getLastSerialno());
    CPPUNIT_ASSERT(recv);
    CPPUNIT_ASSERT(recv->GetTo() == character);
    validate(getArg(recv), protoEntity);
}

Object TestOOG::retrieveAccount(ClientConnection &con)
{
    CPPUNIT_ASSERT(con.isOpen());

    Look look;
    look.SetFrom(con.getAccount());
    
    Object::MapType args;
    args["id"] = con.getAccount();
    
    look.SetArgs(Object::ListType(1, args));
    con.send(look);
  
    RootOperation *sight = c.recv("sight", con.getLastSerialno());
    CPPUNIT_ASSERT_MESSAGE("failed to recv SIGHT response to OOG account look", sight);
    
    // get the account out
    Object account = getArg(sight);
    
    // validate it a bit
    Object::MapType accountProto;
    accountProto["parents"] = Object::ListType(1, "account");
    accountProto["id"] = con.getAccount();
    validate(account, accountProto);
    
    return account;
}
