#ifndef PYHP_PYTHON_PROXY_H
#define PYHP_PYTHON_PROXY_H 1

#include <Python.h>
#include <sapi/embed/php_embed.h>

void pyhp_init_python_object_proxy(void);
void pyhp_create_python_object_proxy(zval *var, PyObject *object);

#endif // PYHP_PYTHON_PROXY_H
