#include <Python.h>
#include <sapi/embed/php_embed.h>
#include "translate_python_value.h"
#include "translate_php_value.h"


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
} pyhp_python_object_proxy_t;



int pyhp_is_python_object_proxy(zval *value) {
    return Z_OBJCE_P(value) == pyhp_ce_python_object_proxy;
}


PyObject *pyhp_get_proxied_python_object(zval *value) {
    pyhp_python_object_proxy_t *proxy = (pyhp_python_object_proxy_t*)zend_object_store_get_object(value TSRMLS_CC);
    return proxy->object;
}


static void free_python_object_proxy(void *object TSRMLS_DC) {
    pyhp_python_object_proxy_t *proxy = (pyhp_python_object_proxy_t*)object;
    Py_XDECREF(proxy->object);
    proxy->object = NULL;
}


static zend_object_value create_pyhp_python_object_proxy_t(zend_class_entry *class_type TSRMLS_DC) {
    zend_object_value retval;
    pyhp_python_object_proxy_t *intern;

    intern = (pyhp_python_object_proxy_t*)emalloc(sizeof(pyhp_python_object_proxy_t));
    memset(intern, 0, sizeof(pyhp_python_object_proxy_t));

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
    int argc = ZEND_NUM_ARGS(), i;
    PyObject *arg_values = PyTuple_New(argc), *object;
    zval **args;

    zend_get_parameters_array_ex(argc, &args);
    for (i = 0; i < argc; ++i) {
        PyTuple_SET_ITEM(arg_values, i, pyhp_translate_php_value(args[i]));
    }

    object = pyhp_get_proxied_python_object(getThis());
    if (object) {
        if (PyCallable_Check(object)) {
            PyObject *result = PyObject_CallObject(object, arg_values);
            zval *ret_val = pyhp_translate_python_value(result);
            Py_XDECREF(result);
            if (ret_val != NULL)
                RETVAL_ZVAL(ret_val, 0, 0);
        } else {
            PyObject *object_str = PyObject_Str(object);
            php_error_docref(NULL TSRMLS_CC, E_NOTICE, "'%s' is not callable", PyString_AsString(object_str));
            Py_XDECREF(object_str);
        }
    }

    Py_DECREF(arg_values);
}


static PHP_METHOD(PythonObjectProxy, __get) {
    PyObject *object;
    char *attr_name;
    int attr_name_length;

    if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &attr_name, &attr_name_length) == FAILURE) || !attr_name_length)
        return;

    object = pyhp_get_proxied_python_object(getThis());
    if (object) {
        PyObject *attr = PyObject_GetAttrString(object, attr_name);
        if (attr) {
            zval *ret_val = pyhp_translate_python_value(attr);
            Py_DECREF(attr);
            if (ret_val != NULL)
                RETVAL_ZVAL(ret_val, 0, 0);
        }
    }
}


static PHP_METHOD(PythonObjectProxy, __call) {
    char *attr_name;
    int attr_name_length;
    zval *args, **arg;
    HashTable *args_hash;
    HashPosition args_pointer;
    PyObject *arg_values, *object;
    int i = 0;

    if ((zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &attr_name, &attr_name_length, &args) == FAILURE) || !attr_name_length)
        return;

    args_hash = Z_ARRVAL_P(args);
    arg_values = PyTuple_New(zend_hash_num_elements(args_hash));
    for (zend_hash_internal_pointer_reset_ex(args_hash, &args_pointer);
         zend_hash_get_current_data_ex(args_hash, (void**)&arg, &args_pointer) == SUCCESS;
         zend_hash_move_forward_ex(args_hash, &args_pointer)) {
        PyTuple_SET_ITEM(arg_values, i++, pyhp_translate_php_value(*arg));
    }

    object = pyhp_get_proxied_python_object(getThis());
    if (object) {
        PyObject *attr = PyObject_GetAttrString(object, attr_name);

        if (PyCallable_Check(attr)) {
            PyObject *result = PyObject_CallObject(attr, arg_values);
            if (result) {
                zval *ret_val = pyhp_translate_python_value(result);
                Py_DECREF(result);
                if (ret_val != NULL)
                    RETVAL_ZVAL(ret_val, 0, 0);
            }
        } else {
            PyObject *attr_str = PyObject_Str(attr);
            php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Attribute %s ('%s') is not callable", attr_name, PyString_AsString(attr_str));
            Py_XDECREF(attr_str);
        }

        Py_XDECREF(attr);
    }

    Py_DECREF(arg_values);
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
    ce.create_object = create_pyhp_python_object_proxy_t;
    pyhp_ce_python_object_proxy = zend_register_internal_class(&ce TSRMLS_CC);
}


void pyhp_create_python_object_proxy(zval *var, PyObject *object) {
    pyhp_python_object_proxy_t *proxy;

    Py_XINCREF(object);

    object_init_ex(var, pyhp_ce_python_object_proxy);

    proxy = (pyhp_python_object_proxy_t*)zend_object_store_get_object(var TSRMLS_CC);
    proxy->object = object;
}
