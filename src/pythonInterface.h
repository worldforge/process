// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000-2003 Alistair Riddoch

#ifndef PROCESS_PYTHON_INTERFACE_H
#define PROCESS_PYTHON_INTERFACE_H

#include <Python.h>

#include <string>
#include <set>

#include <stdio.h>
#include <unistd.h>

#include "opfwd.h"

class ClientConnection;

typedef struct {
    PyObject_HEAD
    PyObject * m_attr;  // Attributes dictionary
    Atlas::Message::Element * m_obj;
} PyMessageElement;

typedef struct {
    PyObject_HEAD
    RootOperation operation;
} PyOperation;

typedef struct {
    PyObject_HEAD
    ClientConnection * connection;
} PyConnection;

extern PyTypeObject PyOperation_Type;
extern PyTypeObject PyMessageElement_Type;
extern PyTypeObject PyConnection_Type;

PyOperation * newPyOperation();
PyMessageElement * newPyMessageElement();

#define PyMessageElement_Check(_o) ((PyTypeObject*)PyObject_Type((PyObject*)_o)==&PyMessageElement_Type)
#define PyOperation_Check(_o) ((PyTypeObject*)PyObject_Type((PyObject*)_o)==&PyOperation_Type)
#define PyConnection_Check(_o) ((PyTypeObject*)PyObject_Type((PyObject*)_o)==&PyOperation_Type)

//
// Utility functions to munge between Object related types and python types
//

PyObject * MessageElement_asPyObject(const Atlas::Message::Element & obj);
Atlas::Message::ListType PyListObject_asElementList(PyObject * list);
Atlas::Message::MapType PyDictObject_asElementMap(PyObject * dict);
Atlas::Message::Element PyObject_asMessageElement(PyObject * o);

void init_python_api();
bool runScript(const std::string &,
           const std::set<ClientConnection *> & = std::set<ClientConnection*>());

#endif // PROCESS_PYTHON_INTERFACE_H
