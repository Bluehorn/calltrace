# -*- coding: utf-8 -*-

"""
Tests to illustrate the shortcomings of sys._current_frames.
"""

from __future__ import print_function
import pytest

import sys
import time
import weakref
import threading
try:
    import queue
except ImportError:
    import Queue as queue

import calltrace


@pytest.fixture
def short_checkinterval(request):
    """
    Sets a small interval using sys.setcheckinterval to cause many context
    switches.
    """
    old_interval = sys.getcheckinterval()
    sys.setcheckinterval(0)
    request.addfinalizer(lambda: sys.setcheckinterval(old_interval))

@pytest.mark.parametrize(("function_name",), [
        pytest.mark.xfail(("sys._current_frames",)),
        ("calltrace.current_frames",)])
def test_current_frames_breaks_unref(function_name, short_checkinterval):
    """
    Illustrates how just calling sys._current_frames can cause problems by
    pulling stack frames to other threads and destructing objects from there.
    """
    RUNTIME_SECONDS = 1
    bad_unrefs_queue = queue.Queue()

    function = eval(function_name)

    stop_event = threading.Event()
    worker = threading.Thread(target=_worker_thread,
            name="Worker Thread",
            args=(stop_event, bad_unrefs_queue))
    worker.start()
    errors = 0
    t0 = time.time()
    try:
        while time.time() < t0 + RUNTIME_SECONDS:
            function()
            try:
                error = bad_unrefs_queue.get(block=False)
                print("bad unref: %s" % (error,))
                errors += 1
            except queue.Empty:
                pass
    finally:
        stop_event.set()
        worker.join()

    print("Deallocations on wrong thread: %s" % (errors,))
    assert errors == 0


def _worker_thread(stop_event, bad_unrefs_queue):
    def fac(n):
        decoy = ThreadSensitiveObject(bad_unrefs_queue)
        return fac(n-1) * n if n > 1 else 1

    while not stop_event.is_set():
        fac(48)


class ThreadSensitiveObject(object):
    """
    An object that must be destructed on the thread where it was created.
    In the original setting this was part of a GUI toolkit and destructing
    the object on the wrong thread made the main loop hang.
    """
    _refs = set()

    def __init__(self, bad_unrefs_queue):
        def unref_callback(ref):
            refs.discard(ref)
            if threading.current_thread() != thread:
                bad_unrefs_queue.put(instance_id)

        instance_id = id(self)
        thread = threading.current_thread()
        refs = ThreadSensitiveObject._refs
        refs.add(weakref.ref(self, unref_callback))
