# -*- coding: utf-8 -*-

import sys
import traceback
from calltrace import CallTrace


def test_trivial_calltrace(tmpdir):
    def make_trace(creator):
        return creator()

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
