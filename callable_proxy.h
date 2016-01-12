#ifndef PYHP_CALLABLE_PROXY_H
#define PYHP_CALLABLE_PROXY_H 1

#include <Python.h>
#include <sapi/embed/php_embed.h>

void pyhp_init_callable_proxy(void);
void pyhp_create_callable_proxy(zval *var, PyObject *callable);

#endif // PYHP_CALLABLE_PROXY_H
