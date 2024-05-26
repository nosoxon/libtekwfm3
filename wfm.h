#pragma once
#include <stdint.h>

enum WfmHdrSetType : uint32_t {
	/* Single waveform set */
	WFM_SET_TYPE_SINGLE_WAVEFORM = 0,
	/* FastFrame set */
	WFM_SET_TYPE_FASTFRAME = 1,
};

enum WfmHdrDataType : uint32_t {
	/* scalar measurement */
	WFM_DATA_TYPE_SCALAR_MEAS = 0,
	/* scalar constant */
	WFM_DATA_TYPE_SCALAR_CONST = 1,
	/* normal YT waveforms */
	WFM_DATA_TYPE_VECTOR = 2,
	/* invalid */
	WFM_DATA_TYPE_INVALID = 4,
	/* for example Waveform database */
	WFM_DATA_TYPE_WFMDB = 5,
	/* digital waveforms from MSO source */
	WFM_DATA_TYPE_DIGITAL = 6,
};

enum WfmHdrSummaryFrame : uint16_t {
	WFM_SUMMARY_FRAME_OFF = 0,
	WFM_SUMMARY_FRAME_AVERAGE = 1,
	WFM_SUMMARY_FRAME_ENVELOPE = 2,
};

enum WfmHdrPixmapDsyFormat : uint32_t {
	WFM_PIXMAP_DSY_FMT_INVALID = 0,
	WFM_PIXMAP_DSY_FMT_YT = 1,
	WFM_PIXMAP_DSY_FMT_XY = 2,
	WFM_PIXMAP_DSY_FMT_XYZ = 3,
};

enum WfmExpDimDescInfoFormat : uint32_t {
	WFM_DATA_FORMAT_INT16 = 0,
	WFM_DATA_FORMAT_INT32 = 1,
	WFM_DATA_FORMAT_UINT32 = 2,
	WFM_DATA_FORMAT_UINT64 = 3,
	WFM_DATA_FORMAT_FP32 = 4,
	WFM_DATA_FORMAT_FP64 = 5,
	/* v.3 exclusive from here on*/
	WFM_DATA_FORMAT_UINT8 = 6, /* INVALID_FORMAT in other versions */
	WFM_DATA_FORMAT_INT8 = 7,
	WFM_DATA_FORMAT_INVALID_FORMAT = 8,
};

enum WfmExpDimDescInfoStorageType : uint32_t {
	WFM_STORAGE_TYPE_SAMPLE = 0,
	WFM_STORAGE_TYPE_MIN_MAX = 1,
	WFM_STORAGE_TYPE_VERT_HIST = 2,
	WFM_STORAGE_TYPE_HOR_HIST = 3,
	WFM_STORAGE_TYPE_ROW_ORDER = 4,
	WFM_STORAGE_TYPE_COLUMN_ORDER = 5,
	WFM_STORAGE_TYPE_INVALID_STORAGE = 6,
};

union WfmExpDimDescInfoNullValue {
	int16_t sRange;
	int32_t lRange;
	float fRange;
};

enum WfmTimeBaseInfoSweep : uint32_t {
	WFM_SWEEP_ROLL = 0,
	WFM_SWEEP_SAMPLE = 1,
	WFM_SWEEP_ET = 2,
	WFM_SWEEP_INVALID = 3,
};

enum WfmTimeBaseInfoType : uint32_t {
	WFM_TIMEBASE_TYPE_TIME = 0,
	WFM_TIMEBASE_TYPE_SPECTRAL_MAG = 1,
	WFM_TIMEBASE_TYPE_SPECTRAL_PHASE = 2,
	WFM_TIMEBASE_TYPE_INVALID = 3,
};

enum WfmCurveSpecChecksumType : uint32_t {
	WFM_CHECKSUM_TYPE_NO_CHECKSUM = 0,
	WFM_CHECKSUM_TYPE_CRC16 = 1,
	WFM_CHECKSUM_TYPE_SUM16 = 2,
	WFM_CHECKSUM_TYPE_CRC32 = 3,
	WFM_CHECKSUM_TYPE_SUM32 = 4,
};

#pragma pack(push, 1)
struct WfmStaticFileInfo {
	/* Byte order verification
	 *
	 * Hex 0xF0F0 (PPC format) or 0x0F0F (Intel). Used to determine
	 * if bytes read from file need to be swapped before processing.
	 */
	uint16_t ByteOrderVerification;
	/* Version number
	 *
	 * The values will correspond to the following characters:
	 * `WFM#001`, where `001` is replaced by the actual file
	 * version. The byte values will be (in hex)
	 *
	 *     `3a 57 46 4d 23 30 30 31` (where `0x303031 == 001`).
	 *
	 * SEE NOTE 6.
	 */
	char VersionNumber[8];
	/* Number of digits in byte count
	 *
	 * 0-9
	 */
	uint8_t ByteCountDigits;
	/* Byte offset to beginning of curve buffer
	 *
	 * Up to 9 digits in length, which allows for a file size of
	 * 999,999,999 bytes. The number of bytes from here to the end
	 * of the file.
	 */
	int32_t BytesRemaining;
	/* Number of bytes per point
	 *
	 * Number of bytes per curve data point. Allows for quick
	 * determination of the size of the actual curve data.
	 */
	uint8_t BytesPerPoint;
	/* Byte offset to beginning of curve buffer
	 *
	 * Allows for positioning of file descriptor for recall of the
	 * curve buffer. The number of bytes from the beginning of the
	 * file to the start of the curve buffer.
	 */
	int32_t CurveBufferOffset;
	/* Horizontal zoom scale factor
	 *
	 * Horizontal scale zoom information.
	 *
	 * Not for use.
	 */
	int32_t HorizontalZoomScaleFactor;
	/* Horizontal zoom position
	 *
	 * Horizontal position zoom information.
	 *
	 * Not for use.
	 */
	float HorizontalZoomPosition;
	/* Vertical zoom scale factor
	 *
	 * Vertical scale zoom information.
	 *
	 * Not for use.
	 */
	double VerticalZoomScaleFactor;
	/* Vertical zoom position
	 *
	 * Vertical position zoom information.
	 *
	 * Not for use.
	 */
	float VerticalZoomPosition;
	/* Waveform label
	 *
	 * User defined label for the reference waveform.
	 */
	char Label[32];
	/* N (number of FastFrames - 1)
	 *
	 * The number of WfmUpdateSpec and Curve objects that follow.
	 * This number is equal to the number of FastFrames – 1 (for
	 * FastFrame waveform sets).
	 */
	uint32_t NFastFrames;
	/* Size of the waveform header
	 *
	 * The size in bytes of the waveform header, which directly
	 * follows this field.
	 */
	uint16_t HdrSize;
};

struct WfmReferenceFileData {
	/* Type of waveform set
	 *
	 * A FastFrame set of 1 frame can be a legal degenerate state
	 * even if FastFrame is off. While early releases always matched
	 * the FastFrame state to the user mode selection, some 5/6
	 * Series MSO revisions treat the default-run-state as a
	 * single-frame FastFrame waveform even if FastFrame is off.
	 */
	enum WfmHdrSetType SetType;
	/* Number of waveforms in the set
	 *
	 * FastFrame is a special case in that it describes a waveform
	 * set of one waveform with multiple frames. See the numFrames
	 * field described below.
	 */
	uint32_t WfmCnt;
	/* Internal acquisition counter
	 *
	 * This is not a time stamp. It is used to make sure that a set
	 * of acquisition waveforms are of the same acquisition. This
	 * counter is the number of acquisitions since power on. If nil
	 * (0), the internal system will match any waveform with this
	 * one.
	 *
	 * Not for use.
	 */
	uint64_t AcquisitionCounter;
	/* Internal acquisition translation stamp
	 *
	 * This is not a time value. It is the time of the transaction
	 * that this waveform is based upon.
	 *
	 * Not for use.
	 */
	uint64_t TransactionCounter;
	/* Slot ID
	 *
	 * An enumeration based upon the number and type of data slots
	 * available in a specified product. These values are subject to
	 * change based upon the need of the specified product.
	 *
	 * Not for use.
	 */
	uint32_t SlotID;
	/* Is static flag
	 *
	 * Used internally to determine if waveform is static (for
	 * example, a reference) or live (for examples, a channel or
	 * math).
	 *
	 * Not for use.
	 */
	int32_t IsStatic;
	/* Waveform update specification count
	 *
	 * Number of wfm update specifications in the waveform set. The
	 * waveform update spec holds data, which changes on each
	 * acquisition update. FastFrame waveform sets have multiple
	 * update specs (for example, each frame has unique timestamps
	 * and such).
	 */
	uint32_t UpdateSpecCount;
	/* Implicit dimension ref count
	 *
	 * The number of implicit dimensions for the given waveform.
	 * Vector Lists (vector YT and XY) waveform sets have one
	 * implicit dimension; Pixel Maps (YT, XY, and XYZ) waveform
	 * sets have one explicit dimension; and Scalars (measurements
	 * and constants) have zero implicit dimensions.
	 */
	uint32_t ImpDimRefCount;
	/* Explicit dimension ref count
	 *
	 * The number of explicit dimensions for the waveform set.
	 * Vector Lists (vector YT, XY, and XYZ) waveforms have one
	 * (YT) or two (XY) explicit dimension(s); Pixel Maps (YT, XY,
	 * and XYZ) waveform sets have two implicit dimensions; and
	 * Scalars (measurements and constants) have zero explicit
	 * dimensions.
	 *
	 * Implicit dimension axis values are determined by a value plus
	 * index times increment. Explicit dimension axis values are
	 * explicitly defined for each data point.
	 */
	uint32_t ExpDimRefCount;
	/* Data type */
	enum WfmHdrDataType DataType;
	/* General purpose counter
	 *
	 * Internal usage only. Definition varies by specific system.
	 *
	 * Not for use.
	 */
	uint64_t GeneralPurposeCounter;
	/* Accumulated waveform count
	 *
	 * Used by the internal Math system. Represents the number of
	 * waveforms that have gone into the accumulation. This value is
	 * updated on a per acquisition basis.
	 *
	 * Not for use.
	 */
	uint32_t AccumulatedWfmCount;
	/* Target accumulation count
	 *
	 * Number of acquisitions requested to be made.
	 *
	 * Not for use.
	 */
	uint32_t TargetAccumulationCount;
	/* Curve ref count
	 *
	 * The number of curve objects for the given waveform set.
	 * Normally 1.
	 */
	uint32_t CurveRefCount;
	/* Number of requested fast frames
	 *
	 * Number of FastFrame acquisitions that were requested for a
	 * given acquisition set.
	 *
	 * Not for use.
	 */
	uint32_t NFastFramesRequested;
	/* Number of acquired fast frames
	 *
	 * The number of frames that the acquisition system actually
	 * acquired. This number will be less than or equal to the
	 * `NFastFramesRequested` (~numRequestedFastFrames~) field.
	 *
	 * Not for use.
	 */
	uint32_t NFastFramesAcquired;
	/* Summary frame */
	enum WfmHdrSummaryFrame SummaryFrame;
	/* Pixmap display format */
	enum WfmHdrPixmapDsyFormat PixmapDsyFormat;
	/* Pixmap max value
	 *
	 * Max value of Pixel map.
	 *
	 * Not for use.
	 */
	uint64_t PixmapMaxValue;
};

struct WfmDimUserView {
	/* User scale
	 *
	 * User view scale expressed in terms of units/divisions and is
	 * used to apply additional scale information for display
	 * purposes.
	 *
	 * Not supported in 5/6 Series MSO instruments.
	 */
	double Scale;
	/* User units
	 *
	 * User display units string, expressed in Units per Division.
	 * Examples are Volts per Div or Time per Div.
	 *
	 * Not supported in 5/6 Series instruments.
	 */
	char Units[20];
	/* User offset
	 *
	 * User position expressed in terms of divisions for the
	 * explicit dimension, and is used to designate the screen
	 * relative position of the waveform.
	 *
	 * Note: For implicit dimension, this value is expressed in
	 * terms of user units (typically time) and is used to designate
	 * the value of the horizontal center pixel column in
	 * relationship to the trigger, in absolute horizontal user
	 * units.
	 *
	 * Not supported in 5/6 Series MSO instruments.
	 */
	double Offset;
	/* Point density [SEE NOTE 6]
	 *
	 * The relationship of screen points to waveform data. For
	 * explicit dimensions, this value is 1.
	 *
	 * Note: For implicit dimensions, this defines the number of
	 * points that are compressed into a single pixel column. This
	 * compression ratio becomes the definition of a zoom of one.
	 *
	 * Not supported in 5/6 Series instruments.
	 */
	double PointDensity;
	/* Horizontal reference
	 *
	 * The horizontal position of the trigger, in units of
	 * percentage of a waveform, in trigger-centric (no delay) mode.
	 * This value is 0% - 100% of the waveform.
	 */
	double HRef;
	/* Trigger delay
	 *
	 * The amount of delay, in seconds, from the trigger to the
	 * HRef location. This value is positive for triggers occurring
	 * before the HRef and negative for values after the HRef.
	 */
	double TrigDelay;
};

struct WfmExpDim {
	/* Dimension scale
	 *
	 * The scale of the waveform data for the given dimension. This
	 * value is used to interpret the curve data for the given axis.
	 * For the vertical (explicit) dimension, scale is used to
	 * calculate voltage.
	 *
	 *    Voltage = (wfmCurveData * Scale) + offset.
	 *
	 * The scale is expressed as volts per LSB of the 16 or 32 bit
	 * digitizer values (or 1.0 for the float digitizer values or
	 * Math waveforms).
	 */
	double Scale;
	/* Dimesion offset
	 *
	 * The distance in units from the dimension's zero value to the
	 * true zero value. This is the ground level offset for the
	 * explicit dimension.
	 */
	double Offset;
	/* Dimension size
	 *
	 * The size of the explicit dimension in terms of the base
	 * storage value. For the explicit dimension, it is the size
	 * of the storage element (e.g. 252 for 8 bit values and 65532
	 * for 16 bit integers).
	 *
	 * Note: Record length computations are made using this field
	 * and the curve byte offsets.
	 */
	uint32_t Size;
	/* Units
	 *
	 * A null-terminated character string which represents the units
	 * for the dimension.
	 */
	char Units[20];
	/* Dimension minimum extent
	 *
	 * The minimum attainable data value for the explicit dimension,
	 * adjusted for scale and offset.
	 *
	 * Not for use.
	 */
	double ExtentMin;
	/* Dimension maximum extent
	 *
	 * The maximum attainable data value for the explicit dimension,
	 * adjusted for scale and offset.
	 *
	 * Not for use.
	 */
	double ExtentMax;
	/* Dimension resolution
	 *
	 * For the explicit dimension, this value reflects the smallest
	 * resolution (of a voltage) possible given the product's
	 * digitizer and acquisition mode, i.e. the value of one
	 * digitizing level (DL).
	 */
	double Resolution;
	/* Dimension reference point
	 *
	 * For the explicit dimension, this is the ground-level
	 * reference value.
	 */
	double RefPoint;
	/* Format
	 *
	 * The code type of data values stored in the curve buffer.
	 */
	enum WfmExpDimDescInfoFormat Format;
	/* Storage type
	 *
	 * Describes the layout of the data values stored in the curve.
	 */
	enum WfmExpDimDescInfoStorageType StorageType;
	/* Null value
	 *
	 * The value that represents the NULL (unacquired) waveform
	 * value. This is the value that will be in the waveform data
	 * when there is no valid data available for that data element.
	 * This is not a number (NAN) for floating point values.
	 *
	 * The 4 bytes can be interpreted as short, long, or float
	 * depending upon the curve data type.
	 *
	 *     union rangeValues {
	 *         int16_t sRange;
	 *         int32_t lRange;
	 *         float fRange;
	 *     };
	 */
	union WfmExpDimDescInfoNullValue NullValue;
	/* Over range
	 *
	 * Special value that indicates that a point is over-ranged.
	 *
	 * Not for use.
	 */
	uint32_t OverRange;
	/* Under range
	 *
	 * Special value that indicates that a point is under-ranged.
	 *
	 * Not for use.
	 */
	uint32_t UnderRange;
	/* High range
	 *
	 * The largest signed value that can be present in this data.
	 * (Except for null value, under/over-range.)
	 *
	 * Not for use.
	 */
	uint32_t HighRange;
	/* Low range
	 *
	 * The smallest value that can be present in this data. (Except
	 * for null value, under/over-range.)
	 *
	 * Not for use.
	 */
	uint32_t LowRange;
	/* The UserView data defines the relationship between the
	 * description of the raw data and the way the user wants to
	 * view and interact with the data. This data defines the
	 * meaning of zoom as well as the transformations that relate
	 * the data to the screen. This can include specific user
	 * modified scale and units values.
	 */
	struct WfmDimUserView UserView;
};

struct WfmImpDim {
	/* Dimension scale
	 *
	 * The scale of the waveform data for the given dimension. This
	 * value is used to interpret the curve data for the given axis.
	 * For the horizontal implicit dimension, scale is used to
	 * specify the sample interval (i.e.: time per point).
	 */
	double Scale;
	/* Dimesion offset
	 *
	 * The distance in units from the dimension's zero value to the
	 * true zero value. For the implicit dimension, offset is the
	 * trigger position (in a delay centric mode).
	 */
	double Offset;
	/* Dimension size
	 *
	 * For the implicit dimension, size is the record length
	 * (including pre and post charge data) and is expressed in data
	 * points, not bytes of storage. Precharge points are usually 16
	 * points and postcharge points are 16. These are used for
	 * internal display interpolation.
	 *
	 * Note: Record length computations are made using this field
	 * and the curve byte offsets.
	 */
	uint32_t Size;
	/* Units
	 *
	 * A null-terminated character string which represents the units
	 * for the dimension.
	 */
	char Units[20];
	/* Dimension minimum extent
	 *
	 * The minimum attainable data value for the explicit dimension,
	 * adjusted for scale and offset.
	 *
	 * Not for use.
	 */
	double ExtentMin;
	/* Dimension maximum extent
	 *
	 * The maximum attainable data value for the explicit dimension,
	 * adjusted for scale and offset.
	 *
	 * Not for use.
	 */
	double ExtentMax;
	/* Dimension resolution
	 *
	 * Not for use in 5/6 Series instruments.
	 */
	double Resolution;
	/* Dimension reference point
	 *
	 * For the implicit dimension, this is the horizontal reference
	 * point of the time base.
	 */
	double RefPoint;
	/* Spacing
	 *
	 * Real time point spacing [SEE NOTE 1].
	 */
	int32_t Spacing;
	/* The UserView data defines the relationship between the
	 * description of the raw data and the way the user wants to
	 * view and interact with the data. This data defines the
	 * meaning of zoom as well as the transformations that relate
	 * the data to the screen. This can include specific user
	 * modified scale and units values.
	 */
	struct WfmDimUserView UserView;
};

struct WfmTimeBaseInfo {
	/* Real point spacing
	 *
	 * This is an integer count of the difference, in points,
	 * between the acquired points. If a waveform has not been
	 * created using interpolation, then this value is one.
	 */
	uint32_t RealPointSpacing;
	/* Sweep
	 *
	 * Type of acquisition.
	 */
	enum WfmTimeBaseInfoSweep Sweep;
	/* Type of base
	 *
	 * Defines the kind of base.
	 */
	enum WfmTimeBaseInfoType Type;
};

struct WfmUpdateSpec {
	/* Real point offset
	 *
	 * Integer offset from the beginning of valid data (held in the
	 * attribute preChargeStart of the Curve info) that indicates
	 * the first acquired, non-interpolated point in the record.
	 */
	uint32_t RealPointOffset;
	/* TT offset
	 *
	 * The time from the point the trigger occurred to the next data
	 * point in the waveform record. This value represents the
	 * fraction of the sample time from the trigger time stamp to
	 * the next sample.
	 */
	double TTOffset;
	/* Frac sec
	 *
	 * The fraction of a second when the trigger occurred [7]. Used
	 * in combination with GMT seconds.
	 */
	double FracSec;
	/* GMT seconds
	 *
	 * The time based upon gmt time that the trigger occurred in
	 * seconds, Frac sec gives the trigger to a fraction of a
	 * second [7].
	 */
	int32_t GMTSec;
};

struct WfmCurveSpec {
	/* State flags
	 *
	 * Internal usage flag only. Is used to indicate validity of
	 * curve buffer data.
	 *
	 *     typedef struct {
	 *         unsigned flagOver  : 2;
	 *         unsigned flagUnder : 2;
	 *         unsigned flagValid : 2;
	 *         unsigned flagNulls : 2;
	 *     } WfmCurveStateFlags;
	 *
	 *     #define WFM_CURVEFLAG_YES   0
	 *     #define WFM_CURVEFLAG_NO    1
	 *     #define WFM_CURVEFLAG_MAYBE 2
	 */
	uint32_t StateFlags;
	/* Type of check sum
	 *
	 * Indicates the algorithm used to calculate the waveform
	 * checksum.
	 *
	 * Not for use.
	 */
	enum WfmCurveSpecChecksumType ChecksumType;
	/* Checksum
	 *
	 * Curve checksum. Currently not implemented. File checksum used
	 * only.
	 */
	int16_t Checksum;
	/* Precharge start offset
	 *
	 * The byte offset from the start of the curve buffer to the
	 * first valid waveform point. This first portion of data from
	 * PrechargeStart to DataStart is intended to be used by an
	 * interpolation algorithm and is not guaranteed to be available
	 * to the oscilloscope user.
	 */
	uint32_t PrechargeStart;
	/* Data start offset
	 *
	 * The byte offset from the beginning of the curve buffer to the
	 * first point of the record available to the oscilloscope user.
	 */
	uint32_t DataStart;
	/* Postcharge start offset
	 *
	 * The byte offset to the point after the last user accessible
	 * waveform point. There is still valid waveform data following,
	 * but that data is intended to be used by an interpolator and
	 * is not guaranteed to be accessible to the oscilloscope user.
	 */
	uint32_t PostchargeStart;
	/* Postcharge stop offset
	 *
	 * The byte offset to the portion of memory just past the last
	 * valid waveform point in the curve. The buffer may extend
	 * beyond this point (and often will), as described above.
	 */
	uint32_t PostchargeStop;
	/* End of curve buffer offset
	 *
	 * This value is used only for Roll mode acquisitions and is not
	 * meant for oscilloscope users. It designates a specific number
	 * of bytes that are to be allocated for the curve buffer, but
	 * not available for curve data except as defined by Roll mode.
	 * When roll is not enabled, this offset is the same as
	 * PostchargeStop.
	 */
	uint32_t CurveBufferEnd;
};

struct WfmHdr {
	struct WfmReferenceFileData Ref;
	/* Usually defines voltage axis */
	struct WfmExpDim ExpDim1;
	struct WfmExpDim ExpDim2;
	/* Usually defines time axis */
	struct WfmImpDim ImpDim1;
	struct WfmImpDim ImpDim2;
	struct WfmTimeBaseInfo TimeBaseInfo1;
	struct WfmTimeBaseInfo TimeBaseInfo2;
	struct WfmUpdateSpec UpdateSpec;
	struct WfmCurveSpec CurveSpec;
};

struct WfmFastFrames {
	/* N x WfmUpdateSpec objects
	 *
	 * This is for FastFrame waveform sets and defines trigger time
	 * stamp data. N = number of frames – 1.
	 */
	struct WfmUpdateSpec *UpdateSpecs; // parent.StaticFileInfo.NFastFrames
	/* N x WfmCurveSpec objects
	 *
	 * Variable This is for FastFrame waveform sets, and defines
	 * curve data offsets for individual frames. N = number of
	 * frames – 1.
	 */
	struct WfmCurveSpec *CurveSpecs; // parent.StaticFileInfo.NFastFrames
};

struct Wfm {
	struct WfmStaticFileInfo *Static;
	struct WfmHdr *Hdr;
	/* Optional */
	struct WfmFastFrames *FastFrames;
	/* Curve buffer [5]
	 *
	 * Contains curve data (inclusive of pre/post charge) for all
	 * waveforms in set in a contiguous block (see notes below).
	 */
	uint8_t *CurveBuffer; // (Static.NFastFrames + 1) * Hdr.CurveSpec.PostchargeStop
	/* Waveform file checksum
	 *
	 * Checksum for the waveform file. The checksum is calculated by
	 * summing all data values from the Waveform header through the
	 * Curve data as unsigned chars.
	 */
	uint64_t Checksum;
	/* Total size of waveform file in bytes */
	uint64_t Size;
};
#pragma pack(pop)
