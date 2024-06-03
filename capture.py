from ctypes import *
from .wfm import Wfm

class Channel(Structure):
    _fields_ = [
        ('min',                        c_double),
        ('max',                        c_double),
        ('base',                       c_double),
        ('top',                        c_double),
        ('midpoint',                   c_double),
        ('rising_frequency',           c_double),
        ('rising_period',              c_double),
        ('falling_frequency',          c_double),
        ('falling_period',             c_double),

        ('nrising',                    c_uint32),
        ('nfalling',                   c_uint32),

        ('_raw_data',                  POINTER(c_int16)),
        ('_data',                      POINTER(c_double)),

        ('_rising_edges',              POINTER(c_uint32)),
        ('_falling_edges',             POINTER(c_uint32)),

        ('_rising_times',              POINTER(c_double)),
        ('_falling_times',             POINTER(c_double)),
    ]

class Frame(Structure):
    _fields_ = [
        ('_ch',                        POINTER(Channel)),
    ]

class Capture(Structure):
    _fields_ = [
        ('nch',                        c_uint32),
        ('nframes',                    c_uint32),

        ('record_bytes',               c_uint32),
        ('record_len',                 c_uint32),
        ('data_len',                   c_uint32),
        ('precharge_len',              c_uint32),
        ('postcharge_len',             c_uint32),

        ('t_scale',                    c_double),
        ('t_offset',                   c_double),

        ('_v_scale',                   POINTER(c_double)),
        ('_v_offset',                  POINTER(c_double)),

        ('_times',                     POINTER(c_double)),
        ('_frames',                    POINTER(Frame)),
        ('_wfms',                      POINTER(POINTER(Wfm))),

        ('_all_channels',              POINTER(Channel)),
        ('_all_data',                  POINTER(c_double)),
    ]

    def wfm(self, n: int) -> Wfm:
        if n > self.nch:
            raise IndexError('list index out of range')
        return self._wfms[n].contents
