#include <Python.h>
#include <sapi/embed/php_embed.h>
#include "translate_python_value.h"
#include "python_object_proxy.h"


zval *
pyhp_translate_python_value(PyObject *value) {
    zval *var = NULL, *var2 = NULL;
    Py_ssize_t pos = 0;
    PyObject *key, *ivalue;

    if (value == NULL) {
        PyErr_SetString(PyExc_ValueError, "Cannot translate NULL value");
        return NULL;
    }

    MAKE_STD_ZVAL(var);

    if (value == Py_None) {
        ZVAL_NULL(var);
    } else if (PyBool_Check(value)) {
        ZVAL_BOOL(var, (value == Py_True) ? 1 : 0);
    } else if (PyFloat_Check(value)) {
        ZVAL_DOUBLE(var, PyFloat_AsDouble(value));
    } else if (PyInt_Check(value)) {
        ZVAL_LONG(var, PyInt_AsLong(value));
    } else if (PyString_Check(value)) {
        ZVAL_STRINGL(var, PyString_AsString(value), PyString_Size(value), 1);
    } else if (PyDict_Check(value)) {
        array_init(var);

        while (PyDict_Next(value, &pos, &key, &ivalue)) {
            if (! PyString_Check(key)) {
                zval_ptr_dtor(&var);
                PyErr_SetString(PyExc_ValueError, "Unsupported key type");
                return NULL;
            }

            var2 = pyhp_translate_python_value(ivalue);
            if (var2 == NULL) {
                zval_ptr_dtor(&var);
                return NULL;
            }

            add_assoc_zval(var, PyString_AsString(key), var2);
        }
    } else if (PyTuple_Check(value)) {
        array_init(var);

        for (pos=0; pos < PyTuple_GET_SIZE(value); ++pos) {
            ivalue = PyTuple_GetItem(value, pos);
            var2 = pyhp_translate_python_value(ivalue);
            if (var2 == NULL) {
                zval_ptr_dtor(&var);
                return NULL;
            }

            add_next_index_zval(var, var2);
        }
    } else if (PyList_Check(value)) {
        array_init(var);

        for (pos=0; pos < PyList_GET_SIZE(value); ++pos) {
            ivalue = PyTuple_GetItem(value, pos);
            var2 = pyhp_translate_python_value(ivalue);
            if (var2 == NULL) {
                zval_ptr_dtor(&var);
                return NULL;
            }

            add_next_index_zval(var, var2);
        }
    } else {
        pyhp_create_python_object_proxy(var, value);
    }

    return var;
}
