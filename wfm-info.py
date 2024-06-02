#!/usr/bin/env python3
import ctypes
import enum
import string
import sys

from wfm import *

def _get_printable_prefix(data: bytearray):
    return (data[:next((i for i, c in enumerate(data)
                        if chr(c) not in string.printable), len(data))]
            .decode().strip())

def dump_wfm_struct(level: int, s: ctypes.Structure) -> None:
    indent = '  '
    for field in s._fields_:
        attr = field[0]
        if attr[0] == '_': # wrapped type
            attr = attr[1:]
        val = getattr(s, attr)
        if isinstance(val, ctypes.Structure):
            print(f'{indent * level + attr}')
            dump_wfm_struct(level + 1, val)
        elif isinstance(val, enum.IntEnum):
            print(f'{indent * level + attr:40}{val.name} ({val})')
        elif isinstance(val, bytes):
            pre = _get_printable_prefix(bytearray(val))
            if len(val) < 256 and all(v == 0 for v in val):
                pre = '<EMPTY>'
            elif len(pre) == 0:
                pre = '<BINARY>'
            print(f'{indent * level + attr:40}{pre[:24]}')
        else:
            print(f'{indent * level + attr:40}{val}')

def main(wfm_path: str) -> None:
    libwfm = ctypes.CDLL('./libtekwfm.so')

    libwfm.map_wfm.argtypes = (ctypes.c_char_p,)
    libwfm.map_wfm.restype = ctypes.POINTER(Wfm)

    libwfm.unmap_wfm.argtypes = (ctypes.POINTER(Wfm),)
    libwfm.unmap_wfm.restype = ctypes.c_int

    p_wfm = libwfm.map_wfm(wfm_path.encode())
    wfm: Wfm = p_wfm.contents

    print(f'Waveform: {wfm_path}')
    dump_wfm_struct(1, wfm)

    libwfm.unmap_wfm(p_wfm)

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('usage: wfm-info TEK_WFM_PATH')
        sys.exit(1)
    main(sys.argv[1])
