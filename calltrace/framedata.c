/* Defines the information that we keep about a PyFrameObject. We would like
   to keep enough to enable traceback like output, but without keeping local
   variables etc. alive. */

typedef struct _frame_data {
    PyCodeObject *f_code;
    int f_lasti;
} FrameData;

static inline void init_frame_data(FrameData *frame_data, PyFrameObject *frame)
{
    frame_data->f_code = frame->f_code;
    Py_INCREF(frame_data->f_code);
    frame_data->f_lasti = frame->f_lasti;
}

static inline void clear_frame_data(FrameData *frame_data)
{
    Py_XDECREF(frame_data->f_code);
    frame_data->f_code = NULL;
}

static inline
PyObject *frame_data_as_tuple(FrameData *frame_data)
{
    PyObject *linecache = calltrace_state()->linecache_mod;
    PyCodeObject *code = frame_data->f_code;
    PyObject *filename = code->co_filename;
    PyObject *function = code->co_name;
    int l = PyCode_Addr2Line(code, frame_data->f_lasti);
    int empty = 0;
    PyObject *lineno = PyLong_FromLong(l);
    PyObject *source = NULL, *result = NULL;

    if (!lineno)
        return NULL;

    /* Now find the source code for the line */
    if (!PyObject_CallMethod(linecache, "checkcache", "O", filename))
        goto error;

    source = PyObject_CallMethod(linecache, "getline", "OOO", filename, lineno, Py_None);
    if (!source)
        goto error;

    empty = PyObject_Not(source);
    if (empty < 0)
        goto error;
    if (empty) {
        Py_DECREF(source);
        source = Py_None;
        Py_INCREF(source);
    } else {
        PyObject *t = PyObject_CallMethod(source, "strip", NULL);
        if (!t)
            goto error;
        Py_DECREF(source);
        source = t;
    }

    result = PyTuple_Pack(4, filename, lineno, function, source);

error:
    Py_XDECREF(lineno);
    Py_XDECREF(source);
    return result;
}
