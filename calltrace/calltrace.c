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
    .sq_length = CallTrace_length,
    .sq_item = CallTrace_item,
};

static PyTypeObject CallTraceType = {
    .tp_name = "calltrace.CallTrace",
    .tp_basicsize = offsetof(CallTraceObject, frames),
    .tp_itemsize = sizeof(FrameData),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_free = PyObject_Del,
    .tp_new = CallTrace_new,
    .tp_dealloc = (destructor) CallTrace_dealloc,
    .tp_doc = "Describes a call stack",
    .tp_as_sequence = &CallTrace_as_sequence,
};
