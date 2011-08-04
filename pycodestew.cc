#include <Python.h>
#include "structmember.h"
#include "codestew.h"

extern "C" {

typedef struct {
    PyObject_HEAD
    Block *block;
} BlockObject;

static PyMemberDef Block_members[1] = {
    {NULL,0,0,0,NULL}  
};

static PyMethodDef Block_methods[] = {
    //{"name", (PyCFunction)Noddy_name, METH_NOARGS, "Return the name, combining the first and last name" },
    {NULL}
};
static PyObject *Block_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  printf("Block new\n");
    BlockObject *self;
    self = (BlockObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        /*self->first = PyString_FromString("");
        if (self->first == NULL)
          {
            Py_DECREF(self);
            return NULL;
          }
        
        self->last = PyString_FromString("");
        if (self->last == NULL)
          {
            Py_DECREF(self);
            return NULL;
          }

        self->number = 0;*/
    }
    return (PyObject *)self;
}

static int Block_init(BlockObject *self, PyObject *args, PyObject *kwds)
{
  printf("Block init\n");
  self->block = new Block();
    /*PyObject *first=NULL, *last=NULL, *tmp;

    static char *kwlist[] = {"first", "last", "number", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOi", kwlist, 
                                      &first, &last, 
                                      &self->number))
        return -1; 

    if (first) {
        tmp = self->first;
        Py_INCREF(first);
        self->first = first;
        Py_XDECREF(tmp);
    }

    if (last) {
        tmp = self->last;
        Py_INCREF(last);
        self->last = last;
        Py_XDECREF(tmp);
    }*/

    return 0;
}

static void Block_dealloc(BlockObject* self)
{
    self->ob_type->tp_free((PyObject*)self);
}

static PyTypeObject BlockType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pycodestew.Block",        /*tp_name*/
    sizeof(BlockObject), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Block_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Block objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Block_methods,             /* tp_methods */
    Block_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Block_init,      /* tp_init */
    0,                         /* tp_alloc */
    Block_new                  /* tp_new */
};

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
    if (PyType_Ready(&BlockType) < 0)
        return;

    m = Py_InitModule3("pycodestew", pycodestew_methods,
                       "Python Interface to CodeStew.");

    Py_INCREF(&BlockType);
    PyModule_AddObject(m, "Block", (PyObject *)&BlockType);
}

}
