#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include <Atlas/Message/Object.h>
#include <cppunit/TestAssert.h>

using namespace Atlas::Message;

bool validate(const Atlas::Message::Object &src,
    const Atlas::Message::Object &templ)
{    
    // basic validation
    CPPUNIT_ASSERT_EQUAL(src.GetType(),templ.GetType());
    if (src.GetType() != templ.GetType())
	return false;
    
try {
    bool error=false;
    
    if (templ.IsMap()) {
	const Object::MapType &srcMap = src.AsMap(),
	    tempMap = templ.AsMap();
	
	for (Object::MapType::const_iterator C=tempMap.begin(); C!=tempMap.end();++C) {
	    Object::MapType::const_iterator S = srcMap.find(C->first);
	    if (S == srcMap.end()) {
		CPPUNIT_ASSERT_MESSAGE("missing attribute " + C->first, false);
		error=true;
		continue;
	    }
	    
	    // isn't recursion cool?
	    if (validate(S->second, C->second))
		error = true;
	}
    }
    
    return error;
}
    catch (...) {
	CPPUNIT_ASSERT_MESSAGE("caught exception in validate()", false);
	return false;
    }
}