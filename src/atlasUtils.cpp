#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "atlasUtils.h"
#include <cppunit/TestAssert.h>

using namespace Atlas::Message;
using namespace Atlas::Objects::Operation;

const Object& getArg(RootOperation *op, int pos)
{
    CPPUNIT_ASSERT(op);
    return op->GetArgs()[pos];
}
