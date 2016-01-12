#ifndef PYHP_TRANSLATE_PHP_VALUE_H
#define PYHP_TRANSLATE_PHP_VALUE_H 1

#include <Python.h>
#include <sapi/embed/php_embed.h>

PyObject *pyhp_translate_php_value(zval *value);

#endif // PYHP_TRANSLATE_PHP_VALUE_H
