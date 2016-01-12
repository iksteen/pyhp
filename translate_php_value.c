#include <Python.h>
#include <sapi/embed/php_embed.h>
#include "translate_php_value.h"
#include "python_object_proxy.h"


zval *
pyhp_translate_php_value(PyObject *value) {
    zval *var = NULL, *var2 = NULL;
    Py_ssize_t pos = 0;
    PyObject *key, *ivalue;

    if (value == Py_None) {
        MAKE_STD_ZVAL(var);
        ZVAL_NULL(var);
    } else if (PyBool_Check(value)) {
        MAKE_STD_ZVAL(var);
        ZVAL_BOOL(var, (value == Py_True) ? 1 : 0);
    } else if (PyFloat_Check(value)) {
        MAKE_STD_ZVAL(var);
        ZVAL_DOUBLE(var, PyFloat_AsDouble(value));
    } else if (PyInt_Check(value)) {
        MAKE_STD_ZVAL(var);
        ZVAL_LONG(var, PyInt_AsLong(value));
    } else if (PyString_Check(value)) {
        MAKE_STD_ZVAL(var);
        ZVAL_STRINGL(var, PyString_AsString(value), PyString_Size(value), 1);
    } else if (PyDict_Check(value)) {
        MAKE_STD_ZVAL(var);
        array_init(var);

        while (PyDict_Next(value, &pos, &key, &ivalue)) {
            if (! PyString_Check(key)) {
                zval_ptr_dtor(&var);
                PyErr_SetString(PyExc_ValueError, "Unsupported key type");
                return NULL;
            }

            var2 = pyhp_translate_php_value(ivalue);
            if (var2 == NULL) {
                zval_ptr_dtor(&var);
                return NULL;
            }

            add_assoc_zval(var, PyString_AsString(key), var2);
        }
    } else if (PyTuple_Check(value)) {
        MAKE_STD_ZVAL(var);
        array_init(var);

        for (pos=0; pos < PyTuple_GET_SIZE(value); ++pos) {
            ivalue = PyTuple_GetItem(value, pos);
            var2 = pyhp_translate_php_value(ivalue);
            if (var2 == NULL) {
                zval_ptr_dtor(&var);
                return NULL;
            }

            add_next_index_zval(var, var2);
        }
    } else if (PyList_Check(value)) {
        MAKE_STD_ZVAL(var);
        array_init(var);

        for (pos=0; pos < PyList_GET_SIZE(value); ++pos) {
            ivalue = PyTuple_GetItem(value, pos);
            var2 = pyhp_translate_php_value(ivalue);
            if (var2 == NULL) {
                zval_ptr_dtor(&var);
                return NULL;
            }

            add_next_index_zval(var, var2);
        }
    } else if (PyCallable_Check(value)) {
        MAKE_STD_ZVAL(var);
        pyhp_create_python_object_proxy(var, value);
    } else {
        PyErr_SetString(PyExc_ValueError, "Unsupported value type");
        var = NULL;
    }

    return var;
}
