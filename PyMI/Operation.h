#pragma once

#include <Python.h>
#include <MI++.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    MI::Operation* operation;
} Operation;

extern PyTypeObject OperationType;

PyObject* Operation_new(PyTypeObject *type, PyObject *args, PyObject *kwds);