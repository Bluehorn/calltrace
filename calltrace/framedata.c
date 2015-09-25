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
