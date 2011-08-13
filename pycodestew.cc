#include <Python.h>
#include "structmember.h"
#include "SimpleMachine.h"

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
    TypeObject *self;
    self = (TypeObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *)self;
}

static int Type_init(TypeObject *self, PyObject *args, PyObject *kwds)
{
  char *kind;
  uint64 size;

  if(!PyArg_ParseTuple(args, "sk", &kind, &size))
    return -1;
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
    ValueObject *self;
    self = (ValueObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *)self;
}

static int Value_init(ValueObject *self, PyObject *args, PyObject *kwds)
{
  char *kind;
  uint64 size;

  if(!PyArg_ParseTuple(args, "sk", &kind, &size))
    return -1;
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
    BlockObject *self;
    self = (BlockObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *)self;
}

static int Block_init(BlockObject *self, PyObject *args, PyObject *kwds)
{
  self->block = new Block();
    return 0;
}

static PyObject *blockInput(PyObject *self, PyObject *args)
{
  PyObject *type;
  if(!PyArg_ParseTuple(args, "O", &type) ||
      type->ob_type != &TypeType)
    return NULL;
  Type *valType = ((TypeObject*)type)->type;
  Block *block = ((BlockObject*)self)->block;
  Value *newVal = block->input(valType);
  ValueObject *result = (ValueObject*)_PyObject_New(&ValueType);
  result->value = newVal;
  return (PyObject*)result;
}

static PyObject *blockOutput(PyObject *self, PyObject *args)
{
  PyObject *value;
  if(!PyArg_ParseTuple(args, "O", &value) ||
      value->ob_type != &ValueType)
    return NULL;
  Value *native = ((ValueObject*)value)->value;
  Block *block = ((BlockObject*)self)->block;
  block->output(native);
  Py_RETURN_NONE;
}

static PyObject *blockDump(PyObject *self)
{
  Block *block = ((BlockObject*)self)->block;
  std::string d = block->dump();
  return Py_BuildValue("s",d.c_str());
}

static PyObject *blockDot(PyObject *self, PyObject *args)
{
char *filename;
  if(!PyArg_ParseTuple(args, "s", &filename))
    return NULL;
  Block *block = ((BlockObject*)self)->block;
  block->dot(filename);
  Py_RETURN_NONE;
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
  {"output", (PyCFunction)blockOutput, METH_VARARGS, "Mark a value as an output in the block"},
  {"dump", (PyCFunction)blockDump, METH_NOARGS, "Dump the VDG structure as text"},
  {"dot", (PyCFunction)blockDot, METH_VARARGS, "Create a graphviz file"},
  {NULL}
};
static PyGetSetDef Block_getseters[] = {
    {NULL}  /* Sentinel */
};

PYTYPE(Block)

typedef struct {
    PyObject_HEAD
    SimpleMachine *machine;
} SimpleMachineObject;

static PyObject *SimpleMachine_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SimpleMachineObject *self;
    self = (SimpleMachineObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
    }
    return (PyObject *)self;
}

static int SimpleMachine_init(SimpleMachineObject *self, PyObject *args, PyObject *kwds)
{
  self->machine = new SimpleMachine();
    return 0;
}

static void SimpleMachine_dealloc(SimpleMachineObject* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyObject *simpleXor(PyObject *self, PyObject *args)
{
  PyObject *block, *left, *right, *outType;
  if(!PyArg_ParseTuple(args, "O(OO)O", &block, &left, &right, &outType) ||
      block->ob_type   != &BlockType ||
      left->ob_type    != &ValueType ||
      right->ob_type   != &ValueType ||
      outType->ob_type != &TypeType)
    return NULL;
  Value *output = ((SimpleMachineObject*)self)->machine->XOR( ((BlockObject*)block)->block, 
                                 ((ValueObject*)left)->value, ((ValueObject*)right)->value);
  ValueObject *result = (ValueObject*)_PyObject_New(&ValueType);
  result->value = output;
  return (PyObject *)result;
}

static PyMemberDef SimpleMachine_members[] = {
  {NULL,0,0,0,NULL}  
};

static PyMethodDef SimpleMachine_methods[] = {
  {"XOR", (PyCFunction)simpleXor, METH_VARARGS, "Create a XOR instruction in a SimpleMachine"},
  {NULL}
};
static PyGetSetDef SimpleMachine_getseters[] = {
    {NULL}  /* Sentinel */
};

PYTYPE(SimpleMachine)

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
    if (PyType_Ready(&SimpleMachineType) < 0)
        return;

    m = Py_InitModule3("pycodestew", pycodestew_methods,
                       "Python Interface to CodeStew.");

    Py_INCREF(&BlockType);
    PyModule_AddObject(m, "Block", (PyObject *)&BlockType);
    PyModule_AddObject(m, "Type", (PyObject *)&TypeType);
    PyModule_AddObject(m, "Value", (PyObject *)&ValueType);
    PyModule_AddObject(m, "SimpleMachine", (PyObject *)&SimpleMachineType);
}

}
