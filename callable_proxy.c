#include <Python.h>
#include <sapi/embed/php_embed.h>


static zend_class_entry *pyhp_ce_callable_proxy;


typedef struct {
    zend_object std;
    PyObject *callable;
} php_callable_proxy_t;


static void free_callable_proxy(void *object TSRMLS_DC) {
    php_callable_proxy_t *proxy = (php_callable_proxy_t*)object;
    Py_XDECREF(proxy->callable);
    proxy->callable = NULL;
}


static zend_object_value create_php_callable_proxy_t(zend_class_entry *class_type TSRMLS_DC) {
    zend_object_value retval;
    php_callable_proxy_t *intern;

    intern = (php_callable_proxy_t*)emalloc(sizeof(php_callable_proxy_t));
    memset(intern, 0, sizeof(php_callable_proxy_t));

    zend_object_std_init(&intern->std, class_type TSRMLS_CC);
    object_properties_init((zend_object*) &(intern->std), class_type);

    retval.handle = zend_objects_store_put(
        intern,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        free_callable_proxy,
        NULL
        TSRMLS_CC
    );
    retval.handlers = zend_get_std_object_handlers();
    return retval;
}


static PHP_METHOD(PythonCallableProxy, __invoke) {
    php_callable_proxy_t *proxy;

    proxy = (php_callable_proxy_t*)zend_object_store_get_object(getThis() TSRMLS_CC);
    if (proxy->callable) {
        PyObject_CallObject(proxy->callable, NULL);
    }
}


static zend_function_entry callable_proxy_methods[] = {
    PHP_ME(PythonCallableProxy, __invoke, NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};


void pyhp_init_callable_proxy(void) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "PythonCallableProxy", callable_proxy_methods);
    ce.create_object = create_php_callable_proxy_t;
    pyhp_ce_callable_proxy = zend_register_internal_class(&ce TSRMLS_CC);
}


void pyhp_create_callable_proxy(zval *var, PyObject *callable) {
    php_callable_proxy_t *proxy;

    Py_XINCREF(callable);

    object_init_ex(var, pyhp_ce_callable_proxy);

    proxy = (php_callable_proxy_t*)zend_object_store_get_object(var TSRMLS_CC);
    proxy->callable = callable;
}
