/* Defines the Python object that represents the whole call trace. Basically,
   this is an array of FrameData entries. */

typedef struct {
    PyObject_VAR_HEAD
    /* Information about all captured frames, ob_size defines length. */
    FrameData frames[1];
} CallTraceObject;

static PyTypeObject CallTraceType;

static inline int frame_depth(PyFrameObject *frame)
{
    int depth = 0;
    while (frame) {
        depth += 1;
        frame = frame->f_back;
    }
    return depth;
}


static CallTraceObject *
CallTrace_from_frame(PyTypeObject *subtype, PyFrameObject *frame)
{
    int depth = frame_depth(frame);
    int i;

    CallTraceObject *self = (CallTraceObject *) subtype->tp_alloc(subtype, depth);
    for (i = 0; i < depth; frame = frame->f_back, i++)
        init_frame_data(&self->frames[depth - i - 1], frame);
    return self;
}

static PyObject *
CallTrace_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds)
{
    /* Always a borrowed reference. Either from caller or from thread state,
       we are unlikely to leave that frame early... */

    PyObject *frame = NULL;

    if (!PyArg_UnpackTuple(args, "CallTrace", 0, 1, &frame))
        return NULL;

    if (!frame)
        frame = (PyObject *) PyThreadState_GET()->frame;

    return (PyObject *) CallTrace_from_frame(subtype, (PyFrameObject *) frame);
}

static void
CallTrace_dealloc(CallTraceObject *self)
{
    int i;

    for (i = 0; i < Py_SIZE(self); i++) {
        clear_frame_data(&self->frames[i]);
    }

    Py_TYPE(self)->tp_free(self);
}

static Py_ssize_t
CallTrace_length(PyObject *self)
{
    return Py_SIZE(self);
}

static PyObject *
CallTrace_item(PyObject *o, Py_ssize_t i)
{
    CallTraceObject *self = (CallTraceObject *) o;
    if (i < 0 || i >= Py_SIZE(o)) {
        PyErr_SetString(PyExc_IndexError, "CallTrace index out of range");
        return NULL;
    }

    return frame_data_as_tuple(&self->frames[i]);
}


static PySequenceMethods CallTrace_as_sequence = {
    CallTrace_length,           /* sq_length */
    0,                          /* sq_concat */
    0,                          /* sq_repeat */
    CallTrace_item,             /* sq_item */
};

static PyTypeObject CallTraceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "calltrace.CallTrace",                      /* tp_name*/
    offsetof(CallTraceObject, frames),          /* tp_basicsize */
    sizeof(FrameData),                          /* tp_itemsize */

    (destructor) CallTrace_dealloc,             /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    &CallTrace_as_sequence,                     /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    "Describes a call stack",                   /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    CallTrace_new,                              /* tp_new */
    PyObject_Del,                               /* tp_free */
};
