#include "stdafx.h"
#include "Class.h"
#include "Utils.h"


PyObject* Class_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    Class* self = NULL;
    self = (Class*)type->tp_alloc(type, 0);
    self->miClass = NULL;
    return (PyObject *)self;
}

static void Class_dealloc(Class* self)
{
    if (self->miClass)
    {
        delete self->miClass;
        self->miClass = NULL;
    }

    self->ob_type->tp_free((PyObject*)self);
}

static PyObject* Class_subscript(Class *self, PyObject *item)
{
    // TODO: size buffer based on actual size
    wchar_t w[1024];
    Py_ssize_t i;
    if (!GetIndexOrName(item, w, i))
        return NULL;

    MI::ClassElement element;
    if (i >= 0)
    {
        element = (*self->miClass)[i];
    }
    else
    {
        element = (*self->miClass)[w];
    }
    return MI2Py(element.m_value, element.m_type, element.m_flags);
}

static Py_ssize_t Class_length(Class *self)
{
    return self->miClass->GetElementsCount();
}

static PyObject* Class_getattro(Class *self, PyObject* name)
{
    PyObject* attr = PyObject_GenericGetAttr((PyObject*)self, name);
    if (attr)
    {
        return attr;
    }

    return Class_subscript(self, name);
}

static PyMemberDef Class_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Class_methods[] = {
    { "__getitem__", (PyCFunction)Class_subscript, METH_O | METH_COEXIST, "" },
    { NULL }  /* Sentinel */
};

static PyMappingMethods Class_as_mapping = {
    (lenfunc)Class_length,
    (binaryfunc)Class_subscript,
    NULL
};

PyTypeObject ClassType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "mi.Class",             /*tp_name*/
    sizeof(Class),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Class_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &Class_as_mapping,      /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    (getattrofunc)Class_getattro, /*tp_getattro*/
    0,                          /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Class objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Class_methods,             /* tp_methods */
    Class_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                          /* tp_init */
    0,                         /* tp_alloc */
    Class_new,                 /* tp_new */
};

