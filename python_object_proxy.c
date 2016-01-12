#include <Python.h>
#include <sapi/embed/php_embed.h>
#include "translate_python_value.h"


static zend_class_entry *pyhp_ce_python_object_proxy;


ZEND_BEGIN_ARG_INFO_EX(arginfo___get, 0, ZEND_RETURN_VALUE, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo___call, 0, ZEND_RETURN_VALUE, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_ARRAY_INFO(0, arguments, 0)
ZEND_END_ARG_INFO()


typedef struct {
    zend_object std;
    PyObject *object;
} php_python_object_proxy_t;


static void free_python_object_proxy(void *object TSRMLS_DC) {
    php_python_object_proxy_t *proxy = (php_python_object_proxy_t*)object;
    Py_XDECREF(proxy->object);
    proxy->object = NULL;
}


static zend_object_value create_php_python_object_proxy_t(zend_class_entry *class_type TSRMLS_DC) {
    zend_object_value retval;
    php_python_object_proxy_t *intern;

    intern = (php_python_object_proxy_t*)emalloc(sizeof(php_python_object_proxy_t));
    memset(intern, 0, sizeof(php_python_object_proxy_t));

    zend_object_std_init(&intern->std, class_type TSRMLS_CC);
    object_properties_init((zend_object*) &(intern->std), class_type);

    retval.handle = zend_objects_store_put(
        intern,
        (zend_objects_store_dtor_t) zend_objects_destroy_object,
        free_python_object_proxy,
        NULL
        TSRMLS_CC
    );
    retval.handlers = zend_get_std_object_handlers();
    return retval;
}


static PHP_METHOD(PythonObjectProxy, __invoke) {
    php_python_object_proxy_t *proxy;

    proxy = (php_python_object_proxy_t*)zend_object_store_get_object(getThis() TSRMLS_CC);
    if (proxy->object) {
        if (PyCallable_Check(proxy->object)) {
            PyObject *result = PyObject_CallObject(proxy->object, NULL);
            zval *ret_val = pyhp_translate_python_value(result);
            Py_XDECREF(result);
            if (ret_val != NULL)
                RETURN_ZVAL(ret_val, 0, 0);
        } else {
            PyObject *object_str = PyObject_Str(proxy->object);
            php_error_docref(NULL TSRMLS_CC, E_NOTICE, "'%s' is not callable", PyString_AsString(object_str));
            Py_XDECREF(object_str);
        }
    }
}


static PHP_METHOD(PythonObjectProxy, __get) {
    php_python_object_proxy_t *proxy;
    char *attr_name;
    int attr_name_length;

    if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &attr_name, &attr_name_length) == FAILURE) || !attr_name_length)
        return;

    proxy = (php_python_object_proxy_t*)zend_object_store_get_object(getThis() TSRMLS_CC);
    if (proxy->object) {
        PyObject *attr = PyObject_GetAttrString(proxy->object, attr_name);
        if (attr) {
            zval *ret_val = pyhp_translate_python_value(attr);
            Py_DECREF(attr);
            if (ret_val == NULL)
                return;
            RETURN_ZVAL(ret_val, 0, 0);
        }
    }
}


static PHP_METHOD(PythonObjectProxy, __call) {
    php_python_object_proxy_t *proxy;
    char *attr_name;
    int attr_name_length;
    zval *arguments;

    if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &attr_name, &attr_name_length, &arguments) == FAILURE) || !attr_name_length)
        return;

    proxy = (php_python_object_proxy_t*)zend_object_store_get_object(getThis() TSRMLS_CC);
    if (proxy->object) {
        PyObject *attr = PyObject_GetAttrString(proxy->object, attr_name);
        if (PyCallable_Check(attr)) {
            PyObject *result = PyObject_CallObject(attr, NULL);
            Py_DECREF(attr);
            if (result) {
                zval *ret_val = pyhp_translate_python_value(result);
                Py_DECREF(result);
                if (ret_val == NULL)
                    return;
                RETURN_ZVAL(ret_val, 0, 0);
            }
        } else {
            PyObject *attr_str = PyObject_Str(attr);
            php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Attribute %s ('%s') is not callable", attr_name, PyString_AsString(attr_str));
            Py_XDECREF(attr_str);
            Py_XDECREF(attr);
        }
    }
}


static zend_function_entry python_object_proxy_methods[] = {
    PHP_ME(PythonObjectProxy, __invoke, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(PythonObjectProxy, __get, arginfo___get, ZEND_ACC_PUBLIC)
    PHP_ME(PythonObjectProxy, __call, arginfo___call, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};


void pyhp_init_python_object_proxy(void) {
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "PythonObjectProxy", python_object_proxy_methods);
    ce.create_object = create_php_python_object_proxy_t;
    pyhp_ce_python_object_proxy = zend_register_internal_class(&ce TSRMLS_CC);
}


void pyhp_create_python_object_proxy(zval *var, PyObject *object) {
    php_python_object_proxy_t *proxy;

    Py_XINCREF(object);

    object_init_ex(var, pyhp_ce_python_object_proxy);

    proxy = (php_python_object_proxy_t*)zend_object_store_get_object(var TSRMLS_CC);
    proxy->object = object;
}
