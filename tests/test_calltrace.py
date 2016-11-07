# -*- coding: utf-8 -*-

import pytest
import sys
import traceback
from calltrace import CallTrace, current_frame
from hypothesis import given, settings
import hypothesis.strategies as st


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


def test_reject_non_frame():
    """The first argument to CallTrace must be a frame."""
    pytest.raises(TypeError, CallTrace, 42)


@settings(max_examples=300)
@given(st.integers(min_value=0, max_value=37),
       st.integers(min_value=-1000, max_value=1000),
       st.integers(min_value=-1000, max_value=1000),
       st.integers(min_value=-7, max_value=13).filter(bool))
def test_slice_calltrace(depth, lower, upper, step):
    """
    Slicing a CallTrace object gives some kind of Sequence and works like
    slicing the a list with the same entries.
    """
    def check():
        assert list(CallTrace())[lower:upper:step] == list(CallTrace()[lower:upper:step])

    deep_call(depth, check)


def deep_call(n, target, *args, **kwargs):
    if n < 0:
        raise ValueError("n must be >= 0 (got {0})".format(n))

    def a(i):
        return target(*args, **kwargs) if i == 0 else b(i - 1)

    def b(i):
        return target(*args, **kwargs) if i == 0 else c(i - 1)

    def c(i):
        return target(*args, **kwargs) if i == 0 else d(i - 1)

    def d(i):
        return target(*args, **kwargs) if i == 0 else e(i - 1)

    def e(i):
        return target(*args, **kwargs) if i == 0 else f(i - 1)

    def f(i):
        return target(*args, **kwargs) if i == 0 else a(i - 1)

    return a(n)
