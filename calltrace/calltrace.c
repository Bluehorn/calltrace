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
CallTrace_from_frame(PyFrameObject *frame)
{
    int depth = frame_depth(frame);
    int i;

    CallTraceObject *self = PyObject_NewVar(CallTraceObject, &CallTraceType, depth);
    for (i = 0; i < depth; frame = frame->f_back, i++)
        init_frame_data(&self->frames[i], frame);
    return self;
}

static void
CallTrace_dealloc(CallTraceObject *self)
{
    int i;

    for (i = 0; i < Py_SIZE(self); i++) {
        clear_frame_data(&self->frames[i]);
    }
}


static PyTypeObject CallTraceType = {
    .tp_name = "calltrace.CallTrace",
    .tp_basicsize = offsetof(CallTraceObject, frames),
    .tp_itemsize = sizeof(FrameData),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_free = PyObject_Del,
    .tp_dealloc = CallTrace_dealloc,
    .tp_doc = "Describes a call stack",
};
