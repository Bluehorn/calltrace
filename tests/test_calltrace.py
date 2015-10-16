# -*- coding: utf-8 -*-

import sys
import traceback
from calltrace import CallTrace, current_frame


def test_trivial_calltrace(tmpdir):
    traces = []
    for creator in traceback.extract_stack, CallTrace:
        traces.append(make_trace(creator))

    formatted = list(map(traceback.format_list, traces))
    tmpdir.join("reference.txt").write("".join(formatted[0]))
    tmpdir.join("replicate.txt").write("".join(formatted[1]))

    # On Python 3.5 (at least) the traceback module adds another stack frame from
    # the new StackExtractor class. Just assume an extra frame is okay when using
    # traceback...
    assert formatted[0] == formatted[1] or formatted[0][:-1] == formatted[1]


def make_trace(creator):
    return creator()


def test_export_frame(tmpdir):
    """
    CallTrace can export it's information as frame-like object.
    """
    formatted = []
    for creator in sys._getframe, current_frame:
        formatted.append(traceback.format_stack(make_trace(creator)))

    tmpdir.join("reference.txt").write("".join(formatted[0]))
    tmpdir.join("replicate.txt").write("".join(formatted[1]))

    # On Python 3.5 (at least) the traceback module adds another stack frame from
    # the new StackExtractor class. Just assume an extra frame is okay when using
    # traceback...
    assert formatted[0] == formatted[1] or formatted[0][:-1] == formatted[1]
