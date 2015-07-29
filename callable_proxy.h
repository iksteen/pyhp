#ifndef PYHP_CALLABLE_PROXH_j
#define PYHP_CALLABLE_PROXY_H 1

void pyhp_init_callable_proxy(void);
void pyhp_create_callable_proxy(zval *var, PyObject *callable);

#endif
