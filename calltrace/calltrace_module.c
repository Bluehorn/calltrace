#include "Python.h"
#include "frameobject.h"  /* need access to PyFrameObject, that's why */

#include <stddef.h>

#ifndef Py_SIZE
#define Py_SIZE(o)          (o->ob_size)
#endif


typedef struct {
    /* Reference to linecache module, initialized with module. */
    PyObject *linecache_mod;
} calltrace_state_t;


#if PY_MAJOR_VERSION >= 3

static PyModuleDef calltrace_module;

static inline calltrace_state_t *
calltrace_state()
{
    return PyModule_GetState(PyState_FindModule(&calltrace_module));
}

#else

static inline calltrace_state_t *
calltrace_state()
{
    static calltrace_state_t global_state;
    return &global_state;
}

#define PyLong_FromLong(x)  PyInt_FromLong(x)
#endif

#include "framedata.c"
#include "calltrace.c"

static PyObject *
calltrace_current_frames(PyObject *dummy, PyObject *args)
{
    PyObject *frames = _PyThread_CurrentFrames();
    PyObject *thread, *frame;
    Py_ssize_t pos = 0;

    while (PyDict_Next(frames, &pos, &thread, &frame)) {
        PyObject *call_trace = (PyObject *) CallTrace_from_frame(&CallTraceType, (PyFrameObject*) frame);
        if (call_trace == NULL) {
            Py_DECREF(frames);
            return NULL;
        }
        if (PyDict_SetItem(frames, thread, call_trace) < 0) {
            Py_DECREF(call_trace);
            Py_DECREF(frames);
            return NULL;
        }
        Py_DECREF(call_trace);
    }

    return frames;
}

static const char module_doc[] = "Extract call traces without frame information";

static PyMethodDef calltrace_methods[] = {
    {"current_frames", calltrace_current_frames, METH_NOARGS,
     "Replacement for sys._current_frames"},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


/* Register CallTrace type, return -1 on error or 0 on success. */
static int
register_CallTrace_type(PyObject *module)
{
    if (PyType_Ready(&CallTraceType) < 0)
        return -1;
    Py_INCREF(&CallTraceType);
    return PyModule_AddObject(module, "CallTrace", (PyObject *)&CallTraceType);
}

static int
calltrace_init(calltrace_state_t *state)
{
    state->linecache_mod = PyImport_ImportModule("linecache");
    return state->linecache_mod ? 0 : -1;
}


#ifdef PyModuleDef_HEAD_INIT

/* Python 3 way of setting up the module */

static PyModuleDef calltrace_module = {
    PyModuleDef_HEAD_INIT,

    .m_name = "calltrace",
    .m_doc = module_doc,
    .m_size = sizeof(calltrace_state_t),
    .m_methods = calltrace_methods,
};


PyMODINIT_FUNC
PyInit_calltrace(void)
{
    PyObject *module;
    calltrace_state_t *state;

    module = PyModule_Create(&calltrace_module);
    if (!module)
        return NULL;
    if (register_CallTrace_type(module) < 0)
        goto error;

    state = PyModule_GetState(module);
    if (calltrace_init(state) < 0)
        goto error;

    return module;

error:
    Py_DECREF(module);
    return NULL;
}

#else

/* Python 2 way of setting up the module */

PyMODINIT_FUNC
initcalltrace(void)
{
    PyObject *module;

    module = Py_InitModule3("calltrace", calltrace_methods, module_doc);
    if (!module)
        return;

    if (register_CallTrace_type(module) < 0)
        goto error;

    if (calltrace_init(calltrace_state()) < 0)
        goto error;

    /* err, sure, how does the caller notice we had an error?
       Checking PyErr_Occurred? */
    return;

error:
    Py_DECREF(module);
    return;
}

#endif
