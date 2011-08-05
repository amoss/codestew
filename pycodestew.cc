#include <Python.h>
#include "structmember.h"
#include "codestew.h"

#define PYTYPE(NAME) static PyTypeObject NAME##Type = { \
    PyObject_HEAD_INIT(NULL) 0, "pycodestew." #NAME,  \
    sizeof(NAME##Object), 0, (destructor)NAME##_dealloc, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, \
    #NAME " objects", 0, 0, 0, 0, 0, 0, \
    NAME##_methods, NAME##_members,  \
    NAME##_getseters, 0, 0, 0, 0, 0, (initproc)NAME##_init, \
    0, NAME##_new };

extern "C" {

typedef struct {
    PyObject_HEAD
    Type *type;
    PyObject *size;
} TypeObject;

static PyObject *Type_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  printf("Type new\n");
    TypeObject *self;
    self = (TypeObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *)self;
}

static int Type_init(TypeObject *self, PyObject *args, PyObject *kwds)
{
  printf("Type init\n");
  char *kind;
  uint64 size;

  if(!PyArg_ParseTuple(args, "sk", &kind, &size))
    return -1;
  printf("%s %llu\n", kind,size);
  if(!strcmp("ubits",kind))
    self->type = new Type(Type::UBITS,size);
  else
    return -1;
  self->size = PyInt_FromLong(size);
  return 0;
}

static void Type_dealloc(TypeObject* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *Type_getsize(TypeObject *self, void *closure)
{
  printf("Get Type.size\n");
  Py_INCREF(self->size);
  return self->size;
}

static PyMemberDef Type_members[2] = {
  //{"size", T_INT, offsetof(TypeObject, size), 0, "type size parameter"},
    {NULL,0,0,0,NULL}  
};

static PyMethodDef Type_methods[] = {
    //{"name", (PyCFunction)Noddy_name, METH_NOARGS, "Return the name, combining the first and last name" },
    {NULL}
};

static PyGetSetDef Type_getseters[] = {
    //{"size", (getter)Type_getsize, (setter)Type_setsize, "Type size parameter", NULL},
    {(char*)"size", (getter)Type_getsize, NULL, (char*)"Type size parameter", NULL},
    {NULL}  /* Sentinel */
};
PYTYPE(Type)

typedef struct {
    PyObject_HEAD
    Value *value;
    PyObject *size;
} ValueObject;

static PyObject *Value_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  printf("Value new\n");
    ValueObject *self;
    self = (ValueObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *)self;
}

static int Value_init(ValueObject *self, PyObject *args, PyObject *kwds)
{
  printf("Value init\n");
  char *kind;
  uint64 size;

  if(!PyArg_ParseTuple(args, "sk", &kind, &size))
    return -1;
  printf("%s %llu\n", kind,size);
  self->size = PyInt_FromLong(size);
  return 0;
}

static void Value_dealloc(ValueObject* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *Value_getsize(ValueObject *self, void *closure)
{
  printf("Get Value.size\n");
  Py_INCREF(self->size);
  return self->size;
}

static PyMemberDef Value_members[2] = {
  //{"size", T_INT, offsetof(ValueObject, size), 0, "type size parameter"},
    {NULL,0,0,0,NULL}  
};

static PyMethodDef Value_methods[] = {
    //{"name", (PyCFunction)Noddy_name, METH_NOARGS, "Return the name, combining the first and last name" },
    {NULL}
};

static PyGetSetDef Value_getseters[] = {
    //{"size", (getter)Value_getsize, (setter)Value_setsize, "Value size parameter", NULL},
    {(char*)"size", (getter)Value_getsize, NULL, (char*)"Value size parameter", NULL},
    {NULL}  /* Sentinel */
};
PYTYPE(Value)

typedef struct {
    PyObject_HEAD
    Block *block;
} BlockObject;

static PyObject *Block_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  printf("Block new\n");
    BlockObject *self;
    self = (BlockObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *)self;
}

static int Block_init(BlockObject *self, PyObject *args, PyObject *kwds)
{
  printf("Block init\n");
  self->block = new Block();
    return 0;
}

static PyObject *blockInput(PyObject *self, PyObject *args)
{
  printf("Block.input\n");
  PyObject *type;
  if(!PyArg_ParseTuple(args, "O", &type) ||
      type->ob_type != &TypeType)
    return NULL;
  Type *valType = ((TypeObject*)type)->type;
  Block *block = ((BlockObject*)self)->block;
  Value *newVal = block->input(valType);
  ValueObject *result = (ValueObject*)_PyObject_New(&ValueType);
  result->value = newVal;
  printf("%s\n", valType->repr().c_str());
  return (PyObject*)result;
}

static void Block_dealloc(BlockObject* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyMemberDef Block_members[] = {
  {NULL,0,0,0,NULL}  
};

static PyMethodDef Block_methods[] = {
  {"input", (PyCFunction)blockInput, METH_VARARGS, "Create an input value in the block"},
  {NULL}
};
static PyGetSetDef Block_getseters[] = {
    {NULL}  /* Sentinel */
};

PYTYPE(Block)

static PyMethodDef pycodestew_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initpycodestew(void) 
{
    PyObject* m;

    BlockType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&TypeType) < 0)
        return;
    if (PyType_Ready(&ValueType) < 0)
        return;
    if (PyType_Ready(&BlockType) < 0)
        return;

    m = Py_InitModule3("pycodestew", pycodestew_methods,
                       "Python Interface to CodeStew.");

    Py_INCREF(&BlockType);
    PyModule_AddObject(m, "Block", (PyObject *)&BlockType);
    PyModule_AddObject(m, "Type", (PyObject *)&TypeType);
    PyModule_AddObject(m, "Value", (PyObject *)&ValueType);
}

}
