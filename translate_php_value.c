#include <Python.h>
#include <sapi/embed/php_embed.h>
#include "translate_php_value.h"


PyObject *pyhp_translate_php_value(zval *value) {
    switch(Z_TYPE_P(value)) {
        case IS_NULL:
            break;
        case IS_LONG:
            return PyLong_FromLong(Z_LVAL_P(value));
        case IS_DOUBLE:
            return PyFloat_FromDouble(Z_DVAL_P(value));
        case IS_BOOL:
            return (Z_LVAL_P(value) == 0) ? Py_False : Py_True;
        case IS_STRING:
            return PyString_FromStringAndSize(Z_STRVAL_P(value), Z_STRLEN_P(value));
        default:
            fprintf(stderr, "Unsupported type (%s)\n", zend_zval_type_name(value));
    }

    Py_INCREF(Py_None);
    return Py_None;
}
