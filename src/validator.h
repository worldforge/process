#ifndef ATLAS_VALIDATOR_H
#define ATLAS_VALIDATOR_H

#include <Atlas/Objects/Root.h>

/** given an object, and a template object, ensure there is a match. This means that
    for each value defined in the template, there is an exact match, and for each
    attribute with no value, and arbitrary value *of that type* must exist in the
    object. */

bool validate(const Atlas::Message::Element &src,
              const Atlas::Message::Element &templ);

bool validate(const Atlas::Objects::Root &src,
              const Atlas::Message::Element &templ);

#endif
