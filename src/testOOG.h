#ifndef PROCESS_TEST_OOG
#define PROCESS_TEST_OOG

#include "ClientConnection.h"

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

class TestOOG : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(TestOOG);
    CPPUNIT_TEST(testCharacterLook);
    CPPUNIT_TEST_SUITE_END();
public:
    
    virtual void setUp();
    virtual void tearDown();

    void testCharacterLook();
    void testTakeCharacter();

    Atlas::Message::Object retrieveAccount(ClientConnection &con);
private:
    ClientConnection c;
};

#endif
