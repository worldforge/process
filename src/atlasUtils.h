#ifndef ATLAS_UTILS_H
#define ATLAS_UTILS_H

#include <Atlas/Message/Object.h>
#include <Atlas/Objects/Operation/RootOperation.h>

const Atlas::Message::Object&
getArg(Atlas::Objects::Operation::RootOperation *op, int pos=0);

#endif	