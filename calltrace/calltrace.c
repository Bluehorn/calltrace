/* Defines the Python object that represents the whole call trace. Basically,
   this is an array of FrameData entries. */

struct _CallTrace_struct {
    PyObject_VAR_HEAD
    /* Information about all captured frames, ob_size defines length.
       Most recent call (which is the current stack frame) last. */
    FrameData frames[1];
};

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

    if (frame) {
        if (!PyFrame_Check(frame))
            return PyErr_Format(PyExc_TypeError, "CallTrace.__new__ requires a frame");
    }
    else
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


static PyObject *
CallTrace_subscript(PyObject *self, PyObject *key)
{
    if (PyIndex_Check(key))
    {
        Py_ssize_t key_value;
        key_value = PyNumber_AsSsize_t(key, PyExc_IndexError);
        if (key_value == -1 && PyErr_Occurred())
            return NULL;
        if (key_value < 0)
            key_value += Py_SIZE(self);
        return CallTrace_item(self, key_value);
    }
    else if (PySlice_Check(key))
    {
        PyObject *result;
        Py_ssize_t start, stop, step, slicelength;

        if (PySlice_GetIndicesEx(key, Py_SIZE(self),
                         &start, &stop, &step, &slicelength) < 0) {
            return NULL;
        }

        result = PyTuple_New(slicelength);
        if (result != NULL) {
            Py_ssize_t pos, cur = start;
            for (pos = 0; pos < slicelength; pos++) {
                PyObject *item = CallTrace_item(self, cur);
                cur += step;

                if (!item) {
                    Py_DECREF(result);
                    result = NULL;
                    break;
                }

                PyTuple_SET_ITEM(result, pos, item);
            }
        }
        return result;
    }
    else
    {
        PyErr_Format(PyExc_TypeError,
                     "indices must be integers or slices, not %.200s",
                     key->ob_type->tp_name);
        return NULL;
    }
}

static PyObject *
CallTrace_export_frame(PyObject *self, PyObject *unused)
{
    size_t top_frame = Py_SIZE((PyObject *) self) - 1;
    return (PyObject*) FrameInfo_from_call_trace((CallTraceObject *) self, top_frame);
}


static PyMethodDef CallTrace_methods[] = {
    { "export_frame", (PyCFunction) CallTrace_export_frame, METH_NOARGS,
        "Return the call trace as a frame-like object." },
    { NULL }                    /* sentinel */
};

static PySequenceMethods CallTrace_as_sequence = {
    CallTrace_length,           /* sq_length */
    0,                          /* sq_concat */
    0,                          /* sq_repeat */
    CallTrace_item,             /* sq_item */
};

static PyMappingMethods CallTrace_as_mapping = {
    CallTrace_length,           /* mp_length */
    CallTrace_subscript,        /* mp_subscript */
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
    &CallTrace_as_mapping,                      /* tp_as_mapping */
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
    CallTrace_methods,                          /* tp_methods */
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
