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
    } else {
	// just use == on the type
	switch (templ.GetType()) {
	case Object::TYPE_INT:
	    CPPUNIT_ASSERT((templ.AsInt() == 0) || (templ == src));
	    break;
	
	case Object::TYPE_FLOAT:
	    CPPUNIT_ASSERT((templ.AsFloat() == 0.0) || (templ == src));
	    break;
	
	case Object::TYPE_STRING:
	    CPPUNIT_ASSERT((templ.AsString() == "") || (templ == src));
	    break;
	
	case Object::TYPE_LIST:
	    CPPUNIT_ASSERT(templ.AsList().empty() || (templ == src));
	    break;
	
	case Object::TYPE_NONE:
	case Object::TYPE_MAP:
	    CPPUNIT_ASSERT_MESSAGE("got bad type in validator",false);
	    return true;
	}
    }
    
    return error;
}
    catch (...) {
	CPPUNIT_ASSERT_MESSAGE("caught exception in validate()", false);
	return false;
    }
}