// This file may be redistributed and modified only under the terms of
// the GNU General Public License (See COPYING for details).
// Copyright (C) 2000 Alistair Riddoch

#include "pythonInterface.h"
#include "ClientConnection.h"

#include <Atlas/Objects/Operation/Action.h>
#include <Atlas/Objects/Operation/Create.h>
#include <Atlas/Objects/Operation/Info.h>
#include <Atlas/Objects/Operation/Look.h>
#include <Atlas/Objects/Operation/Move.h>
#include <Atlas/Objects/Operation/Set.h>
#include <Atlas/Objects/Operation/Sight.h>
#include <Atlas/Objects/Operation/Talk.h>
#include <Atlas/Objects/Operation/Touch.h>
#include <Atlas/Objects/Operation/RootOperation.h>

#include <iostream>

#include <stdio.h>
#include <unistd.h>

using Atlas::Message::Object;
using Atlas::Objects::Root;

/*
 * Beginning of Operation section.
 *
 * This is a python type that wraps up operation objects from
 * Atlas::Objects::Operation namespace.
 *
 */

/*
 * Beginning of Operation methods section.
 */

static PyObject * Operation_SetFrom(RootOperationObject * self, PyObject * args)
{
    // Takes string, returns none
    if (self->operation == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid operation");
        return NULL;
    }
    char * from;
    if (!PyArg_ParseTuple(args, "s", &from)) {
        PyErr_SetString(PyExc_TypeError,"from not a string");
        return NULL;
    }
    self->operation->SetFrom(from);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Operation_SetTo(RootOperationObject * self, PyObject * args)
{
    // Takes string, returns none
    if (self->operation == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid operation");
        return NULL;
    }
    char * to;
    if (!PyArg_ParseTuple(args, "s", &to)) {
        PyErr_SetString(PyExc_TypeError,"to not a string");
        return NULL;
    }
    self->operation->SetTo(to);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Operation_SetArgs(RootOperationObject * self, PyObject * args)
{
    // Takes List, returns none
    if (self->operation == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid operation");
        return NULL;
    }
    PyObject * args_object;
    if (!PyArg_ParseTuple(args, "O", &args_object)) {
        PyErr_SetString(PyExc_TypeError,"args not an object");
        return NULL;
    }
    if (!PyList_Check(args_object)) {
        PyErr_SetString(PyExc_TypeError,"args not a list");
        return NULL;
    }
    Object::ListType argslist;
    for(int i = 0; i < PyList_Size(args_object); i++) {
        AtlasObject * item = (AtlasObject *)PyList_GetItem(args_object, i);
        if (!PyAtlasObject_Check(item)) {
            PyErr_SetString(PyExc_TypeError,"args contains non Atlas Object");
            return NULL;
        }
        
        argslist.push_back(*(item->m_obj));
    }
    self->operation->SetArgs(argslist);

    Py_INCREF(Py_None);
    return Py_None;
}

/*
 * Operation methods structure.
 *
 * Generated from a macro in case we need one for each type of operation.
 *
 */

PyMethodDef RootOperation_methods[] = {
    {"SetFrom",         (PyCFunction)Operation_SetFrom,         METH_VARARGS},
    {"SetTo",           (PyCFunction)Operation_SetTo,           METH_VARARGS},
    {"SetArgs",         (PyCFunction)Operation_SetArgs,         METH_VARARGS},
    {NULL,          NULL}
};


/*
 * Beginning of Operation standard methods section.
 */

static void Operation_dealloc(RootOperationObject *self)
{
        if ((self->own != 0) && (self->operation != NULL)) {
            // Can't delete until I have sorted out bugs with own flag
            delete self->operation;
        }
	PyMem_DEL(self);
}

static PyObject * Operation_getattr(RootOperationObject * self, char * name)
{
    return Py_FindMethod(RootOperation_methods, (PyObject *)self, name);
}

PyTypeObject RootOperation_Type = {
        PyObject_HEAD_INIT(&PyType_Type)
        0,                                      // ob_size
        "Operation",                            // tp_name
        sizeof(RootOperationObject),            // tp_basicsize
        0,                                      // tp_itemsize
        //  methods 
        (destructor)Operation_dealloc,          // tp_dealloc
        0,                                      // tp_print
        (getattrfunc)Operation_getattr,         // tp_getattr
        0,                                      // tp_setattr
        0,                                      // tp_compare
        0,                                      // tp_repr
        0,                                      // tp_as_number
        0,                                      // tp_as_sequence
        0,                                      // tp_as_mapping
        0,                                      // tp_hash
};

/*
 * Beginning of Operation creation functions section.
 */

RootOperationObject * newAtlasRootOperation(PyObject *arg)
{
	RootOperationObject * self;
	self = PyObject_NEW(RootOperationObject, &RootOperation_Type);
	if (self == NULL) {
		return NULL;
	}
	self->operation = NULL;
	self->own = 0;
	return self;
}

static PyMethodDef Object_methods[] = {
	{NULL,          NULL}           /* sentinel */
};

/*
 * Beginning of Object standard methods section.
 */

static void Object_dealloc(AtlasObject *self)
{
    if (self->m_obj != NULL) {
        delete self->m_obj;
    }
    Py_XDECREF(self->Object_attr);
    PyMem_DEL(self);
}

static PyObject * Object_getattr(AtlasObject *self, char *name)
{
    return Py_FindMethod(Object_methods, (PyObject *)self, name);
}

static int Object_setattr( AtlasObject *self, char *name, PyObject *v)
{
    if (self->m_obj == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid object");
        return -1;
    }
    if (self->m_obj->IsMap()) {
        Object::MapType & omap = self->m_obj->AsMap();
        Object v_obj = PyObject_asObject(v);
        if (v_obj.GetType() != Object::TYPE_NONE) {
            omap[name] = v_obj;
            return 0;
        }
    }
    if (self->Object_attr == NULL) {
        self->Object_attr = PyDict_New();
        if (self->Object_attr == NULL) {
            return -1;
        }
    }
    return PyDict_SetItemString(self->Object_attr, name, v);
}

PyTypeObject Object_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,				/*ob_size*/
	"AtlasObject",			/*tp_name*/
	sizeof(AtlasObject),		/*tp_basicsize*/
	0,				/*tp_itemsize*/
	/* methods */
	(destructor)Object_dealloc,	/*tp_dealloc*/
	0,                              /*tp_print*/
	(getattrfunc)Object_getattr,    /*tp_getattr*/
	(setattrfunc)Object_setattr,	/*tp_setattr*/
	0,                              /*tp_compare*/
	0,                              /*tp_repr*/
	0,                              /*tp_as_number*/
	0,                              /*tp_as_sequence*/
	0,                              /*tp_as_mapping*/
	0,                              /*tp_hash*/
};

/*
 * Beginning of Object creation functions section.
 */

AtlasObject * newAtlasObject(PyObject *arg)
{
	AtlasObject * self;
	self = PyObject_NEW(AtlasObject, &Object_Type);
	if (self == NULL) {
		return NULL;
	}
	self->Object_attr = NULL;
	return self;
}

/*
 * Utility functions to munge between Object related types and python types
 */

static PyObject * MapType_asPyObject(const Object::MapType & map)
{
    PyObject * args_pydict = PyDict_New();
    Object::MapType::const_iterator I;
    AtlasObject * item;
    for(I=map.begin();I!=map.end();I++) {
        const std::string & key = I->first;
        item = newAtlasObject(NULL);
        if (item == NULL) {
            PyErr_SetString(PyExc_TypeError,"error creating map");
            return NULL;
        }
        item->m_obj = new Object(I->second);
        // PyDict_SetItem() does not eat the reference passed to it
        PyDict_SetItemString(args_pydict,(char *)key.c_str(),(PyObject *)item);
        Py_DECREF(item);
    }
    return args_pydict;
}

static PyObject * ListType_asPyObject(const Object::ListType & list)
{
    PyObject * args_pylist = PyList_New(list.size());
    Object::ListType::const_iterator I;
    int j=0;
    AtlasObject * item;
    for(I=list.begin();I!=list.end();I++,j++) {
        item = newAtlasObject(NULL);
        if (item == NULL) {
            PyErr_SetString(PyExc_TypeError,"error creating list");
            return NULL;
        }
        item->m_obj = new Object(*I);
        // PyList_SetItem() eats the reference passed to it
        PyList_SetItem(args_pylist, j, (PyObject *)item);
    }
    return args_pylist;
}

PyObject * Object_asPyObject(const Object & obj)
{
    PyObject * ret = NULL;
    switch (obj.GetType()) {
        case Object::TYPE_INT:
            ret = PyInt_FromLong(obj.AsInt());
            break;
        case Object::TYPE_FLOAT:
            ret = PyFloat_FromDouble(obj.AsFloat());
            break;
        case Object::TYPE_STRING:
            ret = PyString_FromString(obj.AsString().c_str());
            break;
        case Object::TYPE_MAP:
            ret = MapType_asPyObject(obj.AsMap());
            break;
        case Object::TYPE_LIST:
            ret = ListType_asPyObject(obj.AsList());
            break;
        default:
            break;
    }
    return ret;
}

Object::ListType PyListObject_asListType(PyObject * list)
{
    Object::ListType argslist;
    AtlasObject * item;
    for(int i = 0; i < PyList_Size(list); i++) {
        item = (AtlasObject *)PyList_GetItem(list, i);
        if (PyAtlasObject_Check(item)) {
            argslist.push_back(*(item->m_obj));
        } else {
            Object o = PyObject_asObject((PyObject*)item);
            if (o.GetType() != Object::TYPE_NONE) {
                argslist.push_back(o);
            }
        }
    }
    return argslist;
}

Object::MapType PyDictObject_asMapType(PyObject * dict)
{
    Object::MapType argsmap;
    AtlasObject * item;
    PyObject * keys = PyDict_Keys(dict);
    PyObject * vals = PyDict_Values(dict);
    for(int i = 0; i < PyDict_Size(dict); i++) {
        PyObject * key = PyList_GetItem(keys, i);
        item = (AtlasObject *)PyList_GetItem(vals, i);
        if (PyAtlasObject_Check(item)) {
            argsmap[PyString_AsString(key)] = *(item->m_obj);
        } else {
            Object o = PyObject_asObject((PyObject*)item);
            if (o.GetType() != Object::TYPE_NONE) {
                argsmap[PyString_AsString(key)] = o;
            }
        }
    }
    Py_DECREF(keys);
    Py_DECREF(vals);
    return argsmap;
}

Object PyObject_asObject(PyObject * o)
{
    if (PyInt_Check(o)) {
        return Object((int)PyInt_AsLong(o));
    }
    if (PyFloat_Check(o)) {
        return Object(PyFloat_AsDouble(o));
    }
    if (PyString_Check(o)) {
        return Object(PyString_AsString(o));
    }
    if (PyList_Check(o)) {
        return Object(PyListObject_asListType(o));
    }
    if (PyDict_Check(o)) {
        return Object(PyDictObject_asMapType(o));
    }
    if (PyTuple_Check(o)) {
        Object::ListType list;
        int i, size = PyTuple_Size(o);
        for(i = 0; i < size; i++) {
            Object item = PyObject_asObject(PyTuple_GetItem(o, i));
            if (item.GetType() != Object::TYPE_NONE) {
                list.push_back(item);
            }
        }
        return Object(list);
    }
    if (PyAtlasObject_Check(o)) {
        AtlasObject * obj = (AtlasObject *)o;
        return *(obj->m_obj);
    }
    if (PyOperation_Check(o)) {
        RootOperationObject * op = (RootOperationObject *)o;
        return op->operation->AsObject();
    }
    return Object();
}

static PyObject * Connection_connect(ConnectionObject * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * host;
    if (!PyArg_ParseTuple(args, "s", &host)) {
        PyErr_SetString(PyExc_TypeError,"send takes one argument");
        return NULL;
    }
    bool ret = self->connection->connect(host);
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject * Connection_login(ConnectionObject * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * name;
    char * passwd;
    if (!PyArg_ParseTuple(args, "ss", &name, &passwd)) {
        PyErr_SetString(PyExc_TypeError,"login takes two string arguments");
        return NULL;
    }
    bool ret = self->connection->login(name, passwd);
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject * Connection_create(ConnectionObject * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * name;
    char * passwd;
    if (!PyArg_ParseTuple(args, "ss", &name, &passwd)) {
        PyErr_SetString(PyExc_TypeError,"create takes two string arguments");
        return NULL;
    }
    bool ret = self->connection->create(name, passwd);
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

static PyObject * Connection_send(ConnectionObject * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    RootOperationObject * op;
    if (!PyArg_ParseTuple(args, "O", &op)) {
        PyErr_SetString(PyExc_TypeError,"send takes one argument");
        return NULL;
    }
    if (!PyOperation_Check(op)) {
        PyErr_SetString(PyExc_TypeError,"send argument must be an op");
        return NULL;
    }
    self->connection->send(*op->operation);

    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject * Connection_waitfor(ConnectionObject * self, PyObject * args)
{
    if (self->connection == NULL) {
        PyErr_SetString(PyExc_TypeError,"invalid connection");
        return NULL;
    }
    char * parents;
    AtlasObject * arg;
    if (!PyArg_ParseTuple(args, "sO", &parents, &arg)) {
        PyErr_SetString(PyExc_TypeError,"waitfor takes two arguments");
        return NULL;
    }
    if (!PyAtlasObject_Check(arg)) {
        PyErr_SetString(PyExc_TypeError,"waitfor argument must be an entity");
        return NULL;
    }
    bool ret = self->connection->waitFor(parents, arg->m_obj->AsMap());
    if (ret) {
        Py_INCREF(Py_True);
        return Py_True;
    } else {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

PyMethodDef Connection_methods[] = {
    {"connect",    (PyCFunction)Connection_connect,      METH_VARARGS},
    {"login",      (PyCFunction)Connection_login,        METH_VARARGS},
    {"create",     (PyCFunction)Connection_create,       METH_VARARGS},
    {"send",       (PyCFunction)Connection_send,         METH_VARARGS},
    {"waitfor",    (PyCFunction)Connection_waitfor,      METH_VARARGS},
    {NULL,         NULL}
};

static void Connection_dealloc(ConnectionObject *self)
{
    PyMem_DEL(self);
}

static PyObject * Connection_getattr(ConnectionObject *self, char *name)
{
    std::cout << "getattr" << name << std::endl << std::flush;
    return Py_FindMethod(Connection_methods, (PyObject *)self, name);
}

PyTypeObject Connection_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,                              /*ob_size*/
	"ConnectionObject",             /*tp_name*/
	sizeof(ConnectionObject),	/*tp_basicsize*/
	0,				/*tp_itemsize*/
	/* methods */
	(destructor)Connection_dealloc,	/*tp_dealloc*/
	0,                              /*tp_print*/
	(getattrfunc)Connection_getattr,/*tp_getattr*/
	0,                              /*tp_setattr*/
	0,                              /*tp_compare*/
	0,                              /*tp_repr*/
	0,                              /*tp_as_number*/
	0,                              /*tp_as_sequence*/
	0,                              /*tp_as_mapping*/
	0,                              /*tp_hash*/
};

ConnectionObject * newConnectionObject(PyObject *arg)
{
	ConnectionObject * self;
	self = PyObject_NEW(ConnectionObject, &Connection_Type);
	if (self == NULL) {
		return NULL;
	}
	return self;
}

static PyObject * entity_new(PyObject * self, PyObject * args, PyObject * kwds)
{
    AtlasObject *o;
    char * id = NULL;
    
    if (!PyArg_ParseTuple(args, "|s", &id)) {
        return NULL;
    }
    Object::MapType omap;
    if (id != NULL) {
        omap["id"] = std::string(id);
    }
    if ((kwds != NULL) && (PyDict_Check(kwds))) {
        PyObject * keys = PyDict_Keys(kwds);
        PyObject * vals = PyDict_Values(kwds);
        if ((keys == NULL) || (vals == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Error in keywords");
            return NULL;
        }
        int i, size=PyList_Size(keys); 
        for(i = 0; i < size; i++) {
            char * key = PyString_AsString(PyList_GetItem(keys, i));
            PyObject * val = PyList_GetItem(vals, i);
            if ((strcmp(key, "parent") == 0) && (PyString_Check(val))) {
                omap["loc"] = Object(std::string(PyString_AsString(val)));
            } else if ((strcmp(key, "type") == 0) && (PyString_Check(val))) {
                omap["parents"] = Object::ListType(1,std::string(PyString_AsString(val)));
            } else {
                Object val_obj = PyObject_asObject(val);
                if (val_obj.GetType() == Object::TYPE_NONE) {
                    fprintf(stderr, "Could not handle %s value in Entity()", key);
                    PyErr_SetString(PyExc_TypeError, "Argument type error to Entity()");
                    Py_DECREF(keys);
                    Py_DECREF(vals);
                    return NULL;
                }
                omap[key] = val_obj;
            }
        }
        Py_DECREF(keys);
        Py_DECREF(vals);
    }

    o = newAtlasObject(args);
    if ( o == NULL ) {
        return NULL;
    }
    o->m_obj = new Object(omap);
    return (PyObject *)o;
}

static inline void addToArgs(Object::ListType & args, PyObject * ent)
{
    if (ent == NULL) {
        return;
    }
    if (PyAtlasObject_Check(ent)) {
        AtlasObject * obj = (AtlasObject*)ent;
        if (obj->m_obj == NULL) {
            fprintf(stderr, "Invalid object in Operation arguments\n");
            return;
        }
        Object o(*obj->m_obj);
        if (o.IsMap() && (obj->Object_attr != NULL)) {
            Object::MapType & ent = o.AsMap();
            Object::MapType ent2 = PyDictObject_asMapType(obj->Object_attr);
            Object::MapType::iterator I = ent2.begin();
            for(; I != ent2.end(); I++) {
                if (ent.find(I->first) != ent.end()) {
                    ent[I->first] = I->second;
                }
            }
        }
        args.push_back(o);
    } else if (PyOperation_Check(ent)) {
        RootOperationObject * op = (RootOperationObject*)ent;
        if (op->operation == NULL) {
            fprintf(stderr, "Invalid operation in Operation arguments\n");
            return;
        }
        args.push_back(op->operation->AsObject());
    } else {
        fprintf(stderr, "Non-entity passed as arg to Operation()\n");
    }
}

static PyObject * operation_new(PyObject * self, PyObject * args, PyObject * kwds)
{
    RootOperationObject * op;

    char * type;
    PyObject * to = NULL;
    PyObject * from = NULL;
    PyObject * arg1 = NULL;
    PyObject * arg2 = NULL;
    PyObject * arg3 = NULL;

    if (!PyArg_ParseTuple(args, "s|OOO", &type, &arg1, &arg2, &arg3)) {
        return NULL;
    }
    op = newAtlasRootOperation(args);
    if (op == NULL) {
        return NULL;
    }
    if (strcmp(type, "sight") == 0) {
        op->operation = new Sight(Sight::Instantiate());
    } else if (strcmp(type, "set") == 0) {
        op->operation = new Set(Set::Instantiate());
    } else if (strcmp(type, "action") == 0) {
        op->operation = new Action(Action::Instantiate());
    } else if (strcmp(type, "create") == 0) {
        op->operation = new Create(Create::Instantiate());
    } else if (strcmp(type, "look") == 0) {
        op->operation = new Look(Look::Instantiate());
    } else if (strcmp(type, "move") == 0) {
        op->operation = new Move(Move::Instantiate());
    } else if (strcmp(type, "talk") == 0) {
        op->operation = new Talk(Talk::Instantiate());
    } else if (strcmp(type, "touch") == 0) {
        op->operation = new Touch(Touch::Instantiate());
    } else if (strcmp(type, "info") == 0) {
        op->operation = new Info(Info::Instantiate());
    } else {
        fprintf(stderr, "NOTICE: Python creating a custom %s op\n", type);
        *op->operation = RootOperation::Instantiate();
        Py_DECREF(op);
        Py_INCREF(Py_None);
        return Py_None;
    }
    op->own = 1;
    if (PyMapping_HasKeyString(kwds, "to")) {
        to = PyMapping_GetItemString(kwds, "to");
        PyObject * to_id;
        if (PyString_Check(to)) {
            to_id = to;
        } else if ((to_id = PyObject_GetAttrString(to, "id")) == NULL) {
            fprintf(stderr, "To was not really an entity, as it had no id\n");
            return NULL;
        }
        if (!PyString_Check(to_id)) {
            fprintf(stderr, "To id is not a string\n");
            return NULL;
        }
        op->operation->SetTo(PyString_AsString(to_id));
    }
    if (PyMapping_HasKeyString(kwds, "from_")) {
        from = PyMapping_GetItemString(kwds, "from_");
        PyObject * from_id;
        if (PyString_Check(from)) {
            from_id = from;
        } else if ((from_id = PyObject_GetAttrString(from, "id")) == NULL) {
            fprintf(stderr, "From was not really an entity, as it had no id\n");
            return NULL;
        }
        if (!PyString_Check(from_id)) {
            fprintf(stderr, "From id is not a string\n");
            return NULL;
        }
        op->operation->SetFrom(PyString_AsString(from_id));
        // FIXME I think I need to actually do something with said value now
    }
    Object::ListType args_list;
    addToArgs(args_list, arg1);
    addToArgs(args_list, arg2);
    addToArgs(args_list, arg3);
    op->operation->SetArgs(args_list);
    return (PyObject *)op;
}

static PyObject * connection_new(PyObject* self, PyObject* args)
{
    ConnectionObject *c;
    if (!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    c = newConnectionObject(args);
    if (c == NULL) {
        return NULL;
    }
    c->connection = new ClientConnection();
    return (PyObject *)c;
}

static PyMethodDef atlas_methods[] = {
    {"Operation",  (PyCFunction)operation_new,	METH_VARARGS|METH_KEYWORDS},
    {"Entity",     (PyCFunction)entity_new,	METH_VARARGS|METH_KEYWORDS},
    {"Connection", (PyCFunction)connection_new,	METH_VARARGS},
    {NULL,		NULL}				/* Sentinel */
};

void init_python_api()
{
    std::string pypath(".");

    setenv("PYTHONPATH", pypath.c_str(), 1);

    Py_Initialize();

    if (Py_InitModule("atlas", atlas_methods) == NULL) {
        fprintf(stderr, "Failed to Create atlas module\n");
        return;
    }
}

bool runScript(const std::string & script)
{
    PyObject * package_name = PyString_FromString(script.c_str());
    PyObject * mod_dict = PyImport_Import(package_name);
    Py_DECREF(package_name);
    if (mod_dict == NULL) {
        std::cerr << "Cld not find python module " << script
                  << std::endl << std::flush;
        PyErr_Print();
        return false;
    }
    PyObject * function = PyObject_GetAttrString(mod_dict, "process");
    Py_DECREF(mod_dict);
    if (function == NULL) {
        std::cerr << "Could not find process function" << std::endl
                  << std::flush;
        PyErr_Print();
        return false;
    }
    if (PyCallable_Check(function) == 0) {
        std::cerr << "It does not seem to be a function at all" << std::endl
                  << std::flush;
        Py_DECREF(function);
        return false;
    }
    PyObject * pyob = PyEval_CallFunction(function, "()");

    if (pyob == NULL) {
        if (PyErr_Occurred() == NULL) {
            std::cerr << "Could not call function" << std::endl << std::flush;
        } else {
            std::cerr << "Reporting python error" << std::endl << std::flush;
            PyErr_Print();
        }
    }
    Py_DECREF(function);
    return true;
}
