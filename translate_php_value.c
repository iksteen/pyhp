#include <Python.h>
#include <sapi/embed/php_embed.h>
#include "translate_php_value.h"
#include "python_object_proxy.h"


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
        case IS_ARRAY: {
            PyObject *ret_val_dict = PyDict_New(), *ret_val_arr = PyList_New(0);
            zval **item;
            HashTable *value_hash = Z_ARRVAL_P(value);
            HashPosition value_pointer;
            int n = 0;

            for (zend_hash_internal_pointer_reset_ex(value_hash, &value_pointer);
                 zend_hash_get_current_data_ex(value_hash, (void**)&item, &value_pointer) == SUCCESS;
                 zend_hash_move_forward_ex(value_hash, &value_pointer)) {
                PyObject *value = pyhp_translate_php_value(*item);
                char *key;
                uint key_len;
                ulong index;

                switch (zend_hash_get_current_key_ex(value_hash, &key, &key_len, &index, 0, &value_pointer)) {
                    case HASH_KEY_IS_STRING: {
                        PyObject *key_object = PyString_FromString(key);
                        PyDict_SetItem(ret_val_dict, key_object, value);
                        Py_DECREF(key_object);

                        if (ret_val_arr) {
                            Py_DECREF(ret_val_arr);
                            ret_val_arr = NULL;
                        }

                        break;
                    }
                    case HASH_KEY_IS_LONG: {
                        PyObject *key_object = PyLong_FromLong(index);
                        PyDict_SetItem(ret_val_dict, key_object, value);
                        Py_DECREF(key_object);

                        if (ret_val_arr && index == n) {
                            PyList_Append(ret_val_arr, value);
                        } else if (ret_val_arr) {
                            Py_DECREF(ret_val_arr);
                            ret_val_arr = NULL;
                        }

                        break;
                    }
                }

                Py_DECREF(value);
                n++;
            }

            if (ret_val_arr) {
                Py_DECREF(ret_val_dict);
                return ret_val_arr;
            }

            return ret_val_dict;
        }
        case IS_OBJECT: {
            if (pyhp_is_python_object_proxy(value)) {
                PyObject *object = pyhp_get_proxied_python_object(value);
                Py_XINCREF(object);
                return object;
            }
        }
        default:
            fprintf(stderr, "Unsupported type (%s)\n", zend_zval_type_name(value));
    }

    Py_INCREF(Py_None);
    return Py_None;
}
