from ctypes import *
from .wfm import Wfm

class PWrapper:
    def __init__(self, ptr, length):
        self.ptr = ptr
        self.length = length

    def __len__(self):
        return self.length

    def __getitem__(self, k):
        if k < 0 or k >= self.length:
            raise IndexError('list index out of range')
        return self.ptr[k]

class PPWrapper(PWrapper):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def __getitem__(self, k):
        if k < 0 or k >= self.length:
            raise IndexError('list index out of range')
        return self.ptr[k].contents

class ChildStrucutre(Structure):
    @property
    def parent(self):
        return self._parent

    @parent.setter
    def parent(self, parent):
        self._parent = parent

class Channel(ChildStrucutre):
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

    @property
    def raw_data(self) -> list[int]:
        return PWrapper(self._raw_data, self._parent.data_len)

    @property
    def data(self) -> list[float]:
        return PWrapper(self._data, self._parent.data_len)

    @property
    def rising_edges(self) -> list[int]:
        return PWrapper(self._rising_edges, self.nrising)

    @property
    def falling_edges(self) -> list[int]:
        return PWrapper(self._falling_edges, self.nfalling)

    @property
    def rising_times(self) -> list[float]:
        return PWrapper(self._rising_times, self.nrising)

    @property
    def falling_times(self) -> list[float]:
        return PWrapper(self._falling_times, self.nfalling)

class Frame(ChildStrucutre):
    _fields_ = [
        ('_ch',                        POINTER(Channel)),
    ]

    @property
    def ch(self) -> list[Channel]:
        return PWrapper(self.ch, self.parent.nch)

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

    @property
    def v_scale(self) -> list[float]:
        return PWrapper(self._v_scale, self.nch)

    @property
    def v_offset(self) -> list[float]:
        return PWrapper(self._v_offset, self.nch)

    @property
    def times(self) -> list[float]:
        return PWrapper(self._times, self.data_len)

    @property
    def frames(self) -> list[Frame]:
        return PWrapper(self._frames, self.nframes)

    @property
    def wfms(self) -> list[Wfm]:
        return PPWrapper(self._wfms, self.nch)

    @property
    def all_channels(self) -> list[Channel]:
        return PWrapper(self._all_channels, self.nch * self.nframes)

    @property
    def all_data(self) -> list[float]:
        return PWrapper(self._all_data, self.nch * self.nframes * self.record_len)
