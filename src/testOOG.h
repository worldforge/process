// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2001-2003 Alistair Riddoch and James Turner

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

    Atlas::Objects::Root retrieveAccount(ClientConnection &con);
private:
    ClientConnection c;
};

#endif
