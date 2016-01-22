#ifndef PYHP_PYTHON_PROXY_H
#define PYHP_PYTHON_PROXY_H 1

#include <Python.h>
#include <sapi/embed/php_embed.h>

void pyhp_init_python_object_proxy(void);
void pyhp_create_python_object_proxy(zval *var, PyObject *object);

int pyhp_is_python_object_proxy(zval *var);
// Returns a BORROWED REFERENCE to the proxied PyObject.
PyObject *pyhp_get_proxied_python_object(zval *var);

#endif // PYHP_PYTHON_PROXY_H
