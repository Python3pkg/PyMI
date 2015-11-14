#include "stdafx.h"
#include "Serializer.h"
#include "PyMI.h"
#include "Instance.h"
#include "Class.h"
#include "Utils.h"


static PyObject* Serializer_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    Serializer* self = NULL;
    self = (Serializer*)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

static void Serializer_dealloc(Serializer* self)
{
    if (self->serializer)
    {
        delete self->serializer;
        self->serializer = NULL;
    }

    self->ob_type->tp_free((PyObject*)self);
}

static int Serializer_init(Serializer* self, PyObject* args, PyObject* kwds)
{
    PyErr_SetString(PyMIError, "Please use Serializer.create_serializer to allocate a Serializer object.");
    return -1;
}

Serializer* Serializer_New(MI::Serializer* serializer)
{
    Serializer* obj = (Serializer*)Serializer_new(&SerializerType, NULL, NULL);
    obj->serializer = serializer;
    return obj;
}

static PyObject* Serializer_self(Serializer *self, PyObject*)
{
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject* Serializer_exit(Serializer* self, PyObject*)
{
    if (!self->serializer->IsClosed())
        self->serializer->Close();
    Py_RETURN_NONE;
}

static PyObject* Serializer_SerializeInstance(Serializer* self, PyObject* args, PyObject* kwds)
{
    PyObject* instance = NULL;
    PyObject* includeClassObj = NULL;
    static char *kwlist[] = { "instance", "include_class", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &instance, &includeClassObj))
        return NULL;

    if (!PyObject_IsInstance(instance, reinterpret_cast<PyObject*>(&InstanceType)))
        return NULL;

    bool includeClass = includeClassObj && PyObject_IsTrue(includeClassObj);

    try
    {
        std::wstring data = self->serializer->SerializeInstance(*((Instance*)instance)->instance, includeClass);
        return PyUnicode_FromWideChar(data.c_str(), data.length());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyObject* Serializer_SerializeClass(Serializer* self, PyObject* args, PyObject* kwds)
{
    PyObject* miClass = NULL;
    PyObject* deepObj = NULL;
    static char *kwlist[] = { "miClass", "deep", NULL };
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &miClass, &deepObj))
        return NULL;

    if (!PyObject_IsInstance(miClass, reinterpret_cast<PyObject*>(&ClassType)))
        return NULL;

    bool deep = deepObj && PyObject_IsTrue(deepObj);

    try
    {
        std::wstring data = self->serializer->SerializeClass(*((Class*)miClass)->miClass, deep);
        return PyUnicode_FromWideChar(data.c_str(), data.length());
    }
    catch (std::exception& ex)
    {
        SetPyException(ex);
        return NULL;
    }
}

static PyMemberDef Serializer_members[] = {
    { NULL }  /* Sentinel */
};

static PyMethodDef Serializer_methods[] = {
    { "serialize_instance", (PyCFunction)Serializer_SerializeInstance, METH_VARARGS | METH_KEYWORDS, "Serializes an instance." },
    { "serialize_class", (PyCFunction)Serializer_SerializeClass, METH_VARARGS | METH_KEYWORDS, "Serializes a class." },
    { "__enter__", (PyCFunction)Serializer_self, METH_NOARGS, "" },
    { "__exit__",  (PyCFunction)Serializer_exit, METH_VARARGS, "" },
    { NULL }  /* Sentinel */
};

PyTypeObject SerializerType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "mi.Serializer",             /*tp_name*/
    sizeof(Serializer),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Serializer_dealloc, /*tp_dealloc*/
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
    "Serializer objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    Serializer_methods,             /* tp_methods */
    Serializer_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Serializer_init,      /* tp_init */
    0,                         /* tp_alloc */
    Serializer_new,                 /* tp_new */
};
