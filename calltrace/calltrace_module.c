#include "Python.h"

static PyObject *
calltrace_current_frames(PyObject *dummy, PyObject *args)
{
    PyObject *frames = _PyThread_CurrentFrames();
    Py_XDECREF(frames);
    Py_RETURN_NONE;
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
