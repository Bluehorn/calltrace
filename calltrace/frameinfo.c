struct _FrameInfo_struct {
    PyObject_HEAD

    /* Calltrace object with the extracted information of all frames. */
    CallTraceObject *call_trace;

    /* Index into frames for the exposed frame. */
    Py_ssize_t frame_index;
};

static PyTypeObject FrameInfoType;

static FrameInfoObject *
FrameInfo_from_call_trace(CallTraceObject *call_trace, Py_ssize_t frame_index)
{
    FrameInfoObject *self = NULL;
    if (frame_index >= Py_SIZE((PyObject *) call_trace)) {
        PyErr_Format(PyExc_IndexError,
                "frame index %zu out of range [0..%zd]",
                frame_index, Py_SIZE((PyObject *) call_trace) - 1);
        return NULL;
    }

    self = (FrameInfoObject *) FrameInfoType.tp_alloc(&FrameInfoType, 0);
    if (!self)
        return NULL;

    Py_INCREF(call_trace);
    self->call_trace = call_trace;
    self->frame_index = frame_index;
    return self;
}

static PyObject *
FrameInfo_get_f_back(FrameInfoObject *self, void *unused_closure)
{
    CallTraceObject *call_trace = self->call_trace;
    Py_ssize_t frame_index = self->frame_index;
    if (frame_index > 0)
        return (PyObject *) FrameInfo_from_call_trace(call_trace, frame_index - 1);
    else
        Py_RETURN_NONE;
}

static PyObject *
FrameInfo_get_f_code(FrameInfoObject *self, void *unused_closure)
{
    CallTraceObject *call_trace = self->call_trace;
    PyObject *code = (PyObject *) call_trace->frames[self->frame_index].f_code;
    Py_INCREF(code);
    return code;
}

static PyObject *
FrameInfo_get_f_lineno(FrameInfoObject *self, void *unused_closure)
{
    CallTraceObject *call_trace = self->call_trace;
    return frame_data_lineno(&call_trace->frames[self->frame_index]);
}

static PyObject *
FrameInfo_get_f_globals(FrameInfoObject *self, void *unused_closure)
{
    /* FIXME: We do not capture globals currently, so this does not really work. */
    Py_RETURN_NONE;
}

static void
FrameInfo_dealloc(FrameInfoObject *self)
{
    Py_CLEAR(self->call_trace);
    Py_TYPE(self)->tp_free(self);
}


static PyGetSetDef FrameInfo_getset[] = {
    { "f_back", (getter) FrameInfo_get_f_back, NULL,
      "Links back to the previous frame" },
    { "f_code", (getter) FrameInfo_get_f_code, NULL,
      "Code object used in the stack frame" },
    { "f_lineno", (getter) FrameInfo_get_f_lineno, NULL,
      "Retrieve line number from stack frame" },
    { "f_globals", (getter) FrameInfo_get_f_globals, NULL,
      "Global variables dict from stack frame" },
    {NULL}  /* Sentinel */
};


static PyTypeObject FrameInfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "calltrace.FrameInfo",                      /* tp_name*/
    sizeof(FrameInfoObject),                    /* tp_basicsize */
    0,                                          /* tp_itemsize */

    (destructor) FrameInfo_dealloc,             /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    "Describes a frame of a call stack",        /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    FrameInfo_getset,                           /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
    PyObject_Del,                               /* tp_free */
};
