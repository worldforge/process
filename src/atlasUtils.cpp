// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2002-2003 Alistair Riddoch and James Turner

#ifdef HAVE_CONFIG_H
    #include "config.h"
#endif

#include "atlasUtils.h"

#include "process_debug.h"

#include <Atlas/Objects/RootOperation.h>

#include <iostream>

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
