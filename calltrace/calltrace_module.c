#include "Python.h"
#include "frameobject.h"  /* need access to PyFrameObject, that's why */

#include <stddef.h>

#include "framedata.c"
#include "calltrace.c"

#ifndef Py_SIZE
#define Py_SIZE(o)	(o->ob_size)
#endif


static PyObject *
calltrace_current_frames(PyObject *dummy, PyObject *args)
{
    PyObject *frames = _PyThread_CurrentFrames();
    PyObject *thread, *frame;
    Py_ssize_t pos = 0;

    while (PyDict_Next(frames, &pos, &thread, &frame)) {
        PyObject *call_trace = (PyObject *) CallTrace_from_frame((PyFrameObject*) frame);
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

#ifdef PyModuleDef_HEAD_INIT

/* Python 3 way of setting up the module */

static PyModuleDef calltrace_module = {
    PyModuleDef_HEAD_INIT,
    "calltrace",
    module_doc,
    0,
    calltrace_methods, NULL, NULL, NULL, NULL
};


PyMODINIT_FUNC
PyInit_calltrace(void)
{
    PyObject *module;
    module = PyModule_Create(&calltrace_module);
    return module;
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
}

#endif
