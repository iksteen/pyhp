#include <Python.h>
#include <sapi/embed/php_embed.h>
#include "python_object_proxy.h"
#include "translate_python_value.h"


/* Getting in bed with the devil, try 1. */


/* Global reference count on the PHP environment. */
static int env_refcount = 0;


/* Global variables to capture stdout from the embedded PHP interpreter. */
static char *stdout_buf = NULL;
static size_t stdout_buf_len;


static int
pyhp_set_php_vars(PyObject *d)
{
    zval *var;
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    while (PyDict_Next(d, &pos, &key, &value)) {
        if (! PyString_Check(key)) {
            PyErr_SetString(PyExc_ValueError, "Unsupported key type");
            return -1;
        }

        if ((var = pyhp_translate_python_value(value)) == NULL)
            return -1;

        ZEND_SET_SYMBOL(EG(active_symbol_table), PyString_AS_STRING(key), var);
    }
    return 0;
}


static PyObject *
pyhp_evaluate_or_execute(int mode, PyObject *self, PyObject *args)
{
    char *script, *tmp_buf;
    size_t tmp_buf_len;
    int zend_ret = 0;
    PyObject *d, *rv;
    zend_file_handle file_handle;
#ifdef ZTS
    void ***tsrm_ls;
#endif

    if (!PyArg_ParseTuple(args, "sO!", &script, &PyDict_Type, &d))
        return NULL;

    /* Prepare stdout buffer. */
    tmp_buf = stdout_buf;
    tmp_buf_len = stdout_buf_len;
    stdout_buf = malloc(1);
    if (stdout_buf == NULL) {
        PyErr_NoMemory();
        return NULL;
    }
    stdout_buf[0] = 0;
    stdout_buf_len = 0;

    if (env_refcount++ == 0) {
        php_embed_init(0, NULL PTSRMLS_CC);
        pyhp_init_python_object_proxy();
    }

    zend_first_try {
        /* Set PHP variables. */
        if (pyhp_set_php_vars(d)) {
            if (stdout_buf != NULL)
                free(stdout_buf);
            return NULL;
        }

        /* Evaluate PHP script. */
        if (mode == 0) {
            zend_ret = zend_eval_string(script, NULL, "" TSRMLS_CC);
        } else {
            file_handle.type = ZEND_HANDLE_FILENAME;
            file_handle.filename = script;
            file_handle.free_filename = 0;
            file_handle.opened_path = NULL;
            zend_ret = php_execute_script( &file_handle TSRMLS_CC );
        }
    } zend_catch {
    } zend_end_try();

    if (--env_refcount == 0) {
        php_embed_shutdown(TSRMLS_C);
    }

    /* Create return value from stdout buffer. */
    if (stdout_buf == NULL) {
        rv = NULL;
    } else {
        /* If error occurs, rv will be NULL. */
        if (zend_ret == FAILURE) {
            PyErr_SetString(PyExc_RuntimeError, "PHP script failed to execute");
            rv = NULL;
        } else {
            rv = Py_BuildValue("s", stdout_buf);
        }
        free(stdout_buf);
    }

    stdout_buf = tmp_buf;
    stdout_buf_len = tmp_buf_len;

    return rv;
}

static PyObject *
pyhp_evaluate(PyObject *self, PyObject *args) {
    return pyhp_evaluate_or_execute(0, self, args);
}

static PyObject *
pyhp_execute(PyObject *self, PyObject *args) {
    return pyhp_evaluate_or_execute(1, self, args);
}


static PyMethodDef PyhpMethods[] = {
    {"evaluate",  pyhp_evaluate, METH_VARARGS,
     "Evaluate PHP script."},
    {"execute",  pyhp_execute, METH_VARARGS,
     "Execute PHP script."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static int embed_ub_write(const char *str, unsigned int str_length TSRMLS_DC) {
    if(str_length > 0) {
        stdout_buf = realloc(stdout_buf, stdout_buf_len + str_length + 1);
        if (stdout_buf == NULL) {
            PyErr_NoMemory();
            return 0;
        }

        memcpy(stdout_buf + stdout_buf_len, str, str_length);
        stdout_buf_len += str_length;
        stdout_buf[stdout_buf_len] = 0;
    }

    return str_length;
}


PyMODINIT_FUNC
init_pyhp(void)
{
    (void) Py_InitModule("_pyhp", PyhpMethods);
    php_embed_module.ub_write = embed_ub_write;
}
