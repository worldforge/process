#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "debug.h"
#include "validator.h"
#include "process.h"
#include "testOOG.h"
#include "atlasUtils.h"

#include <Atlas/Objects/Operation/Look.h>

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
    
    // send an account LOOK, to get it back
    Look look;
    look.SetFrom(c.getAccount());
    look.SetSerialno(898);
    
    Object::MapType args;
    args["id"] = c.getAccount();
    
    look.SetArgs(Object::ListType(1, args));
    c.send(look);
  
    
    RootOperation *sight = c.recv("sight", 898);
    CPPUNIT_ASSERT_MESSAGE("failed to recv SIGHT response to OOG account look", sight);
    
    // get the account out
    Object account = getArg(sight);
    
    // validate it a bit
    Object::MapType accountProto;
    accountProto["parents"] = Object::ListType(1, "account");
    accountProto["id"] = c.getAccount();
    validate(account, accountProto);
    
    Object::ListType chars = account.AsMap()["characters"].AsList();
    CPPUNIT_ASSERT(!chars.empty());

    // issue a look for character zero
    args["id"] = chars.front();	// first char ID
    look.SetArgs(Object::ListType(1, args));
    look.SetSerialno(899);
    
    c.send(look);
    RootOperation *recv = c.recv("sight", 899);
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
    // steal account look code
    
    
}