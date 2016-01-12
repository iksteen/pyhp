#ifndef PYHP_TRANSLATE_PYTHON_VALUE_H
#define PYHP_TRANSLATE_PYTHON_VALUE_H 1

#include <Python.h>
#include <sapi/embed/php_embed.h>

zval *pyhp_translate_python_value(PyObject *value);

#endif // PYHP_TRANSLATE_PYTHON_VALUE_H
