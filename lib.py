from ctypes import *
from ctypes import _Pointer as Pointer
from typing import Optional

from .wfm import Wfm
from .capture import Capture

class TekWfm:
    lib: Optional[CDLL] = None

    @staticmethod
    def load_library(lib: Optional[CDLL] = None,
                     lib_path: str = './libtekwfm.so'):
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

        TekWfm.lib = lib

    @classmethod
    def make_capture(cls, wfm_paths: list[str],
                     debounce: float) -> Pointer:
        cargs = cls._get_cargs(wfm_paths)
        p_capture = cls.lib.make_capture(*cargs, debounce)
        if not p_capture:
            raise Exception('failed to make_capture')

        capture = p_capture.contents
        for f in range(capture.nframes):
            capture.frames[f].parent = capture
        for i in range(capture.nframes * capture.nch):
            capture.all_channels[i].parent = capture
        return p_capture

    @classmethod
    def destroy_capture(cls, p_capture: Pointer) -> int:
        if cls.lib.destroy_capture(p_capture) != 0:
            raise Exception('failed to destroy_capture')

    @classmethod
    def _get_cargs(cls, paths: str):
        return len(paths), (c_char_p * len(paths))(
             *(p.encode() for p in paths))
