# -*- coding: utf-8 -*-

import sys
import traceback
from calltrace import CallTrace


def test_trivial_calltrace():
    def make_trace(creator):
        return creator()

    traces = []
    for creator in traceback.extract_stack, CallTrace:
        traces.append(make_trace(creator))

    formatted = list(map(traceback.format_list, traces))
    assert formatted[0] == formatted[1]
