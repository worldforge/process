#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "atlasUtils.h"
#include "process_debug.h"

#include <cppunit/TestAssert.h>

#include <iostream>

using Atlas::Message::Element;
using Atlas::Objects::Root;
using Atlas::Objects::Operation::RootOperation;

const Root getArg(const RootOperation & op, int pos)
{
    // CPPUNIT_ASSERT(op);
    if (!op) {
        verbose( std::cerr << "ERROR: Null operation passed to getArg"
                           << std::endl << std::flush; );
        return Root(0);
    }
    return op->getArgs()[pos];
}
