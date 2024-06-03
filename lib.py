from ctypes import *
from ctypes import _Pointer as Pointer
from typing import Optional

from .wfm import Wfm
from .capture import Capture

_lib: Optional[CDLL] = None

def load_library(lib: Optional[CDLL] = None,
                 lib_path: str = './libtekwfm.so'):
    global _lib
    if lib is None:
        lib = CDLL(lib_path)

    # wfm.h
    lib.map_wfm.argtypes = (c_char_p,)
    lib.map_wfm.restype = POINTER(Wfm)

    lib.unmap_wfm.argtypes = (POINTER(Wfm),)
    lib.unmap_wfm.restype = c_int

    # capture.h
    lib.make_capture.argtypes = (c_int, POINTER(c_char_p), c_double)
    lib.make_capture.restype = POINTER(Capture)

    lib.destroy_capture.argtypes = (POINTER(Capture),)
    lib.destroy_capture.restype = c_int

    _lib = lib

def make_capture(wfm_paths: list[str], debounce: float) -> Pointer:
    cargs = _get_cargs(wfm_paths)
    p_capture = _lib.make_capture(*cargs, debounce)
    if not p_capture:
        raise Exception('failed to make_capture')

    capture = p_capture.contents
    _pointers_to_lists(capture, {
        'v_scale':         capture.nch,
        'v_offset':        capture.nch,
        'times':           capture.data_len,
        'frames':          capture.nframes,
        # 'all_channels':  capture.nch * capture.nframes,
        # 'all_data':      capture.nch * capture.nframes * capture.record_len,
    })
    for frame in capture.frames:
        _pointers_to_lists(frame, {'ch': capture.nch})
        for ch in frame.ch:
            _pointers_to_lists(ch, {
                'raw_data':        capture.data_len,
                'data':            capture.data_len,
                'rising_edges':    ch.nrising,
                'falling_edges':   ch.nfalling,
                'rising_times':    ch.nrising,
                'falling_times':   ch.nfalling,
            })
    return p_capture, capture

def destroy_capture(p_capture: Pointer) -> int:
    if _lib.destroy_capture(p_capture) != 0:
        raise Exception('failed to destroy_capture')

def _get_cargs(paths: list[str]) -> tuple:
    return len(paths), (c_char_p * len(paths))(
            *(p.encode() for p in paths))

def _pointers_to_lists(container, lengths: dict[str, int]):
    for attr, length in lengths.items():
        ptr = getattr(container, '_' + attr)
        setattr(container, attr, [ptr[i] for i in range(length)])
