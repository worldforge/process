// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000 Alistair Riddoch

#ifndef RULESETS_PY_OPERATION_H
#define RULESETS_PY_OPERATION_H

#include <stdio.h>
#include <unistd.h>
#include <Python.h>

#include "opfwd.h"

class ClientConnection;

typedef struct {
    PyObject_HEAD
    PyObject			* Object_attr;	// Attributes dictionary
    Atlas::Message::Element	* m_obj;
} AtlasObject;

typedef struct {
    PyObject_HEAD
    RootOperation	* operation;
    int			own;
} RootOperationObject;

typedef struct {
    PyObject_HEAD
    ClientConnection	* connection;
} ConnectionObject;

extern PyTypeObject RootOperation_Type;
extern PyTypeObject Object_Type;
extern PyTypeObject Connection_Type;

RootOperationObject * newAtlasRootOperation(PyObject *arg);
AtlasObject * newAtlasObject(PyObject *arg);

#define PyAtlasObject_Check(_o) ((PyTypeObject*)PyObject_Type((PyObject*)_o)==&Object_Type)
#define PyOperation_Check(_o) ((PyTypeObject*)PyObject_Type((PyObject*)_o)==&RootOperation_Type)
#define PyConnection_Check(_o) ((PyTypeObject*)PyObject_Type((PyObject*)_o)==&RootOperation_Type)

//
// Utility functions to munge between Object related types and python types
//

PyObject * Object_asPyObject(const Atlas::Message::Element & obj);
Atlas::Message::Element::ListType PyListObject_asListType(PyObject * list);
Atlas::Message::Element::MapType PyDictObject_asMapType(PyObject * dict);
Atlas::Message::Element PyObject_asObject(PyObject * o);

void init_python_api();
bool runScript(const std::string &);

#endif // RULESETS_PY_OPERATION_H
