#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "validator.h"

#include <Atlas/Message/Element.h>
#include <Atlas/Objects/Root.h>

#include <cppunit/TestAssert.h>

#include <iostream>

using Atlas::Message::Element;
using Atlas::Objects::Root;

// FIXME Write a Root validator, using ron's holy lucky super iterators

bool validate(const Root &src,
              const Element &templ)
{
    // basic validation
    CPPUNIT_ASSERT_EQUAL(templ.getType(),Element::TYPE_MAP);
    if (templ.getType() != Element::TYPE_MAP) {
        return false;
    }
 
    try {
        bool error = false;

        const Element::MapType & tempMap = templ.asMap();

        Element::MapType::const_iterator C = tempMap.begin();
        for (; C != tempMap.end(); ++C) {
            try {
                Element S = src->getAttr(C->first);

                if (validate(S, C->second)) {
                    error = true;
                }
            }
            catch (Atlas::Objects::NoSuchAttrException n) {
                std::cerr << "ERROR: Missing attribute " << n.getName()
                          << std::endl << std::flush;
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

bool validate(const Element &src, const Element &templ)
{    
    // basic validation
    CPPUNIT_ASSERT_EQUAL(src.getType(),templ.getType());
    if (src.getType() != templ.getType())
        return false;
    
try {
    bool error=false;
    
    if (templ.isMap()) {
        const Element::MapType &srcMap = src.asMap(),
            tempMap = templ.asMap();
        
        for (Element::MapType::const_iterator C=tempMap.begin(); C!=tempMap.end();++C) {
            Element::MapType::const_iterator S = srcMap.find(C->first);
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
        switch (templ.getType()) {
        case Element::TYPE_INT:
            CPPUNIT_ASSERT((templ.asInt() == 0) || (templ == src));
            break;
        
        case Element::TYPE_FLOAT:
            CPPUNIT_ASSERT((templ.asFloat() == 0.0) || (templ == src));
            break;
        
        case Element::TYPE_STRING:
            CPPUNIT_ASSERT((templ.asString() == "") || (templ == src));
            break;
        
        case Element::TYPE_LIST:
            CPPUNIT_ASSERT(templ.asList().empty() || (templ == src));
            break;
        
        case Element::TYPE_NONE:
        case Element::TYPE_PTR:
        case Element::TYPE_MAP:
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
