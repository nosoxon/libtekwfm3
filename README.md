# libtekwfm3
Full ("not used" members included) Tektronix WFM v.3 C headers and Python
`ctypes` bindings, with reference documentation inline.

## Native Library (optional)
I have included a native library that loads a WFM with `mmap` as an
example and quickstart. However, the definitions in `wfm.py` and `wfm.h`
will work no matter how you load your waveform data (you will have to
implement something similar to `map_wfm()` in `wfm.c`).

## Documentation
Every struct member and enumeration item in `wfm.h` is documented in detail.
All documentation is copied nearly verbatim from the Tektronix-published WFM
Format Reference, with some slight modifications.

## Sample program
Shows almost all fields included in the header and bindings.
```
$ make
gcc -shared -fPIC -O3 -s -o libwfm.so wfm.c

$ ./wfm-info.py ch3.wfm
Waveform: ch3.wfm
  Static
    ByteOrderVerification               3855
    VersionNumber                       :WFM#003
    ByteCountDigits                     7
    BytesRemaining                      3302777
    BytesPerPoint                       2
    CurveBufferOffset                   54784
    HorizontalZoomScaleFactor           1
    HorizontalZoomPosition              0.0
    VerticalZoomScaleFactor             1.0
    VerticalZoomPosition                0.0
    Label                               <EMPTY>
    NFastFrames                         999
    HdrSize                             512
  Hdr
    Ref
      SetType                           FASTFRAME (1)
      WfmCnt                            1
      AcquisitionCounter                0
      TransactionCounter                0
      SlotID                            5
      IsStatic                          0
      UpdateSpecCount                   1
      ImpDimRefCount                    1
      ExpDimRefCount                    1
      DataType                          VECTOR (2)
      GeneralPurposeCounter             0
      AccumulatedWfmCount               1
      TargetAccumulationCount           1
      CurveRefCount                     1
      NFastFramesRequested              1000
      NFastFramesAcquired               1000
      SummaryFrame                      OFF (0)
      PixmapDsyFormat                   INVALID (0)
      PixmapMaxValue                    0
    ExpDim1
      Scale                             4.1015625e-05
      Offset                            0.8505000000000001
      Size                              0
      Units                             V
      ExtentMin                         0.0
      ExtentMax                         0.0
      Resolution                        1.0
      RefPoint                          0.0
      Format                            INT16 (0)
      StorageType                       SAMPLE (0)
      NullValue                         <wfm.WfmExpDimDescInfoNullValue object at 0x723b241348d0>
      OverRange                         0
      UnderRange                        0
      HighRange                         0
      LowRange                          0
      UserView
        Scale                           0.407391796875
        Units                           V
        Offset                          -2.1117834202525025
        PointDensity                    1.0
        HRef                            50.0
        TrigDelay                       0.0
    ExpDim2
      Scale                             0.0
      Offset                            0.0
      Size                              0
      Units                             <EMPTY>
      ExtentMin                         0.0
      ExtentMax                         0.0
      Resolution                        0.0
      RefPoint                          0.0
      Format                            INVALID_FORMAT (9)
      StorageType                       INVALID_STORAGE (6)
      NullValue                         <wfm.WfmExpDimDescInfoNullValue object at 0x723b24134850>
      OverRange                         0
      UnderRange                        0
      HighRange                         0
      LowRange                          0
      UserView
        Scale                           0.0
        Units                           <EMPTY>
        Offset                          0.0
        PointDensity                    0.0
        HRef                            0.0
        TrigDelay                       0.0
    ImpDim1
      Scale                             8e-11
      Offset                            -6.239999999999999e-08
      Size                              1560
      Units                             s
      ExtentMin                         0.0
      ExtentMax                         0.0
      Resolution                        0.0
      RefPoint                          0.0
      Spacing                           0
      UserView
        Scale                           1.2479999999999998e-08
        Units                           s
        Offset                          0.0
        PointDensity                    1.0
        HRef                            50.0
        TrigDelay                       -6.23475e-08
    ImpDim2
      Scale                             0.0
      Offset                            0.0
      Size                              0
      Units                             <EMPTY>
      ExtentMin                         0.0
      ExtentMax                         0.0
      Resolution                        0.0
      RefPoint                          0.0
      Spacing                           0
      UserView
        Scale                           0.0
        Units                           <EMPTY>
        Offset                          0.0
        PointDensity                    0.0
        HRef                            0.0
        TrigDelay                       0.0
    TimeBaseInfo1
      RealPointSpacing                  1
      Sweep                             SAMPLE (1)
      Type                              TIME (0)
    TimeBaseInfo2
      RealPointSpacing                  0
      Sweep                             INVALID (3)
      Type                              INVALID (3)
    UpdateSpec
      RealPointOffset                   0
      TTOffset                          0.65625
      FracSec                           0.334003403705
      GMTSec                            1716334508
    CurveSpec
      StateFlags                        162
      ChecksumType                      NO_CHECKSUM (0)
      Checksum                          0
      PrechargeStart                    0
      DataStart                         64
      PostchargeStart                   3184
      PostchargeStop                    3248
      CurveBufferEnd                    3248
  FastFrames
    UpdateSpecs                         <wfm.LP_WfmUpdateSpec object at 0x723b241349d0>
    CurveSpecs                          <wfm.LP_WfmCurveSpec object at 0x723b24134950>
  CurveBuffer                           <BINARY>
  Checksum                              450274367
  Size                                  3302871
```