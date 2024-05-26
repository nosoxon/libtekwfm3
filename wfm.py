from ctypes import *
from enum import IntEnum


class WfmHdrSetType(IntEnum):
    SINGLE_WAVEFORM = 0
    FASTFRAME = 1


class WfmHdrDataType(IntEnum):
    SCALAR_MEAS = 0
    SCALAR_CONST = 1
    VECTOR = 2
    INVALID = 4
    WFMDB = 5
    DIGITAL = 6


class WfmHdrSummaryFrame(IntEnum):
    OFF = 0
    AVERAGE = 1
    ENVELOPE = 2

class WfmHdrPixmapDsyFormat(IntEnum):
    INVALID = 0
    YT = 1
    XY = 2
    XYZ = 3

class WfmExpDimDescInfoFormat(IntEnum):
    INT16 = 0
    INT32 = 1
    UINT32 = 2
    UINT64 = 3
    FP32 = 4
    FP64 = 5
    # WFM v.3 exclusive from here on
    UINT8 = 6 # INVALID_FORMAT in other versions
    INT8 = 7
    UNKNOWN = 8
    INVALID_FORMAT = 9

class WfmExpDimDescInfoStorageType(IntEnum):
    SAMPLE = 0
    MIN_MAX = 1
    VERT_HIST = 2
    HOR_HIST = 3
    ROW_ORDER = 4
    COLUMN_ORDER = 5
    INVALID_STORAGE = 6

class WfmExpDimDescInfoNullValue(Union):
    _pack_ = 1
    _fields_ = [
        ('sRange',                     c_int16),
        ('lRange',                     c_int32),
        ('fRange',                     c_float),
    ]

class WfmTimeBaseInfoSweep(IntEnum):
    ROLL = 0
    SAMPLE = 1
    ET = 2
    INVALID = 3

class WfmTimeBaseInfoType(IntEnum):
    TIME = 0
    SPECTRAL_MAG = 1
    SPECTRAL_PHASE = 2
    INVALID = 3

class WfmCurveSpecChecksumType(IntEnum):
    NO_CHECKSUM = 0
    CRC16 = 1
    SUM16 = 2
    CRC32 = 3
    SUM32 = 4

class WfmStaticFileInfo(Structure):
    _pack_ = 1
    _fields_ = [
        ('ByteOrderVerification',      c_uint16),
        ('VersionNumber',              c_char * 8),
        ('ByteCountDigits',            c_uint8),
        ('BytesRemaining',             c_int32),
        ('BytesPerPoint',              c_uint8),
        ('CurveBufferOffset',          c_int32),
        ('HorizontalZoomScaleFactor',  c_int32),
        ('HorizontalZoomPosition',     c_float),
        ('VerticalZoomScaleFactor',    c_double),
        ('VerticalZoomPosition',       c_float),
        ('Label',                      c_char * 32),
        ('NFastFrames',                c_uint32),
        ('HdrSize',                    c_uint16),
    ]


class WfmReferenceFileData(Structure):
    _pack_ = 1
    _fields_ = [
        ('_SetType',                   c_uint32),
        ('WfmCnt',                     c_uint32),
        ('AcquisitionCounter',         c_uint64),
        ('TransactionCounter',         c_uint64),
        ('SlotID',                     c_uint32),
        ('IsStatic',                   c_int32),
        ('UpdateSpecCount',            c_uint32),
        ('ImpDimRefCount',             c_uint32),
        ('ExpDimRefCount',             c_uint32),
        ('_DataType',                  c_uint32),
        ('GeneralPurposeCounter',      c_uint64),
        ('AccumulatedWfmCount',        c_uint32),
        ('TargetAccumulationCount',    c_uint32),
        ('CurveRefCount',              c_uint32),
        ('NFastFramesRequested',       c_uint32),
        ('NFastFramesAcquired',        c_uint32),
        ('_SummaryFrame',              c_uint16),
        ('_PixmapDsyFormat',           c_uint32),
        ('PixmapMaxValue',             c_uint64),
    ]

    @property
    def SetType(self):
        return WfmHdrSetType(self._SetType)

    @property
    def DataType(self):
        return WfmHdrDataType(self._DataType)

    @property
    def SummaryFrame(self):
        return WfmHdrSummaryFrame(self._SummaryFrame)

    @property
    def PixmapDsyFormat(self):
        return WfmHdrPixmapDsyFormat(self._PixmapDsyFormat)

class WfmDimUserView(Structure):
    _pack_ = 1
    _fields_ = [
        ('Scale',                      c_double),
        ('Units',                      c_char * 20),
        ('Offset',                     c_double),
        ('PointDensity',               c_double),
        ('HRef',                       c_double),
        ('TrigDelay',                  c_double),
    ]

class WfmExpDim(Structure):
    _pack_ = 1
    _fields_ = [
        ('Scale',                      c_double),
        ('Offset',                     c_double),
        ('Size',                       c_uint32),
        ('Units',                      c_char * 20),
        ('ExtentMin',                  c_double),
        ('ExtentMax',                  c_double),
        ('Resolution',                 c_double),
        ('RefPoint',                   c_double),
        ('_Format',                    c_uint32),
        ('_StorageType',               c_uint32),
        ('NullValue',                  WfmExpDimDescInfoNullValue),
        ('OverRange',                  c_uint32),
        ('UnderRange',                 c_uint32),
        ('HighRange',                  c_uint32),
        ('LowRange',                   c_uint32),
        ('UserView',                   WfmDimUserView),
    ]

    @property
    def Format(self):
        return WfmExpDimDescInfoFormat(self._Format)

    @property
    def StorageType(self):
        return WfmExpDimDescInfoStorageType(self._StorageType)

class WfmImpDim(Structure):
    _pack_ = 1
    _fields_ = [
        ('Scale',                      c_double),
        ('Offset',                     c_double),
        ('Size',                       c_uint32),
        ('Units',                      c_char * 20),
        ('ExtentMin',                  c_double),
        ('ExtentMax',                  c_double),
        ('Resolution',                 c_double),
        ('RefPoint',                   c_double),
        ('Spacing',                    c_int32),
        ('UserView',                   WfmDimUserView),
    ]

class WfmTimeBaseInfo(Structure):
    _pack_ = 1
    _fields_ = [
        ('RealPointSpacing',           c_uint32),
        ('_Sweep',                     c_uint32),
        ('_Type',                      c_uint32),
    ]

    @property
    def Sweep(self):
        return WfmTimeBaseInfoSweep(self._Sweep)

    @property
    def Type(self):
        return WfmTimeBaseInfoType(self._Type)

class WfmUpdateSpec(Structure):
    _pack_ = 1
    _fields_ = [
        ('RealPointOffset',            c_uint32),
        ('TTOffset',                   c_double),
        ('FracSec',                    c_double),
        ('GMTSec',                     c_int32),
    ]

class WfmCurveSpec(Structure):
    _pack_ = 1
    _fields_ = [
        ('StateFlags',                 c_uint32),
        ('_ChecksumType',              c_uint32),
        ('Checksum',                   c_int16),
        ('PrechargeStart',             c_uint32),
        ('DataStart',                  c_uint32),
        ('PostchargeStart',            c_uint32),
        ('PostchargeStop',             c_uint32),
        ('CurveBufferEnd',             c_uint32),
    ]

    @property
    def ChecksumType(self):
        return WfmCurveSpecChecksumType(self._ChecksumType)

class WfmHdr(Structure):
    _pack_ = 1
    _fields_ = [
        ('Ref',                        WfmReferenceFileData),
        ('ExpDim1',                    WfmExpDim),
        ('ExpDim2',                    WfmExpDim),
        ('ImpDim1',                    WfmImpDim),
        ('ImpDim2',                    WfmImpDim),
        ('TimeBaseInfo1',              WfmTimeBaseInfo),
        ('TimeBaseInfo2',              WfmTimeBaseInfo),
        ('UpdateSpec',                 WfmUpdateSpec),
        ('CurveSpec',                  WfmCurveSpec),
    ]

class WfmFastFrames(Structure):
    _pack_ = 1
    _fields_ = [
        # len = StaticFileInfo.NFastFrames
        ('UpdateSpecs',                POINTER(WfmUpdateSpec)),
        # len = StaticFileInfo.NFastFrames
        ('CurveSpecs',                 POINTER(WfmCurveSpec)),
    ]
    # TODO wrap with class that has OOB

class Wfm(Structure):
    _pack_ = 1
    _fields_ = [
        ('_Static',                    POINTER(WfmStaticFileInfo)),
        ('_Hdr',                       POINTER(WfmHdr)),
        ('_FastFrames',                POINTER(WfmFastFrames)),
        # len = (StaticFileInfo.NFastFrames + 1) * Hdr.CurveSpec.PostchargeStop
        ('CurveBuffer',                c_char_p),
        ('Checksum',                   c_uint64),
        ('Size',                       c_uint64),
    ]

    @property
    def Static(self) -> WfmStaticFileInfo:
        return self._Static.contents

    @property
    def Hdr(self) -> WfmHdr:
        return self._Hdr.contents

    @property
    def FastFrames(self) -> WfmFastFrames:
        return self._FastFrames.contents
