/* Copyright (c) 2015, 2025, Oracle and/or its affiliates.*/
/* All rights reserved.*/

/*
   NAME
     jznev.h - JSON Event readers/writers

   DESCRIPTION
     An abstract interface for getting or putting events to/from event
     sources and sinks, e.g. JSON parser, BSON decoder, OSON decoder,
     as well as JSON printer, BSON encoder, OSON encoder.

   RELATED DOCUMENTS
 
   EXPORT FUNCTION(S)
     This declares data structures and macros for the following:
     - JSON text parser
     - JSON event source interface
       This is a pull interface for getting JSON events from a source.
       Sources currently include the JSON parser, the BSON decoder, and
       the OSON decoder.
     - JSON event writer interface
       This is a push interface for putting JSON events to an output.
       Current this includes a print facility for JSON (serializer),
       as well as a BSON encoder.
     - JSON print serializer
       This declares an object and methods for serializing JSON as text.

   INTERNAL FUNCTION(S)

   EXAMPLES

   NOTES
     This is a public (to customers) header file and world-viewable.
     The file can include only public headers.

   MODIFIED   (MM/DD/YY)
   apfwkr      06/23/24 - CI# 36762361 of dmcmahon_vec231221 from main
   apfwkr      06/03/24 - CI# 36691636 of dmcmahon_bug-36598093 from main
   dmcmahon    11/05/24 - Bug 37249695: ShortValue/UshortValue
   dmcmahon    09/04/24 - Bug 37028481: jznuStreamWriter APIs
   dmcmahon    08/30/24 - Bug 37012240: JZNU_PRINT_CANONICAL_DECIMAL
   dmcmahon    05/17/24 - Bug 36598093: JZNU_PRINT_BADVALS
   dmcmahon    01/31/24 - Bug 36249408: JZNU_PRINT_EXPONENTIAL flag
   dmcmahon    12/30/23 - Bug 36148907: JZNU_PRINT_DOUBLE flag
   zliu        12/18/23 - bug 36132471: vector type into json xdk 
   dmcmahon    03/07/23 - Bug 35113192: xmlctx parameter for TSTZ formatting
   dmcmahon    02/06/23 - Bug 35008075: Expose jznuFormatFloat
   jvalera     12/06/22 - Bug 34868963: Add jznuSetEJSONMode
   dmcmahon    08/18/22 - Bug 34511143: Add JZN_ALLOW_BIND_PATHS
   rodfuent    05/31/22 - Bug 34229501: Add JZN_ALLOW_PATH_TO_PATH 
   sriksure    03/11/22 - Bug 33952373: Add JZN_CSID
   dmcmahon    03/09/22 - Bug 33940527: wrapped print
   dmcmahon    02/02/22 - Bug 33820835: new path parser constructor flags
   dmcmahon    01/27/22 - Bug 33793347: add JZNU_PRINT_BASE64
   dmcmahon    09/13/21 - Bug 33349497: add JZN_REPORT_LOCATION
   dmcmahon    06/08/21 - Bug 32973954: add jznParserGetSize()
   dmcmahon    04/08/21 - Bug 32746362: add jznEventQueueToSource()
   dmcmahon    02/25/21 - Bug 32555763: add jznuPrintGetError()
   dmcmahon    02/16/21 - Bug 32528049: obsolete JZN_ORAMEMCTX
   dmcmahon    01/14/21 - Bug 32383277: add jznParserGetPosition
   dmcmahon    01/04/21 - Bug 32340636: add support for JSON comments
   dmcmahon    12/08/20 - Bug 32249674: jznuPrintCreateEx, jznParserCreateEx
   dmcmahon    12/03/20 - Add jznEventQueueEjsonDetect()
   sriksure    10/09/20 - Bug 31996145: Add JsonLoadDom(), JsonSerializeDom()
   dmcmahon    09/15/20 - Bug 31892579: toggle validate-only mode
   dmcmahon    08/12/20 - Bug 31748678: add jznuSetPrintCtxFlags
   dmcmahon    07/13/20 - Add JZN_BLKSIZE, jznParserFromSource()
   dmcmahon    07/01/20 - Move the JSON parser here from jzn.h
   dmcmahon    06/08/20 - Add event source flag JZN_PRINT_ERRORS
   dmcmahon    05/22/20 - jznuPrintCreateMem should be private
   dmcmahon    05/18/20 - Added jznuPrintRecordDirect()
   dmcmahon    05/04/20 - Bug 31292369: Constructors may return NULLs
   dmcmahon    08/16/19 - add JZN_PATH_ARRAY_MODE
   dmcmahon    08/16/19 - Bug 30193262: Add jznuPrintSetOutputCharset
   dmcmahon    04/26/19 - bug 29700066: jznuPrintGetWarning
   dmcmahon    04/24/19 - bug 29687763: JZNU_PRINT_REPLACEMENT
   jvalera     02/25/19 - bug 29379558: Add JZN_OSON_CONFIG_SIZE
   sriksure    02/25/19 - Bug 29405036: Add EncodeAll(), DecodeAll()
   dmcmahon    02/07/19 - Bug 29318213: expose jznuDurationToString
   dmcmahon    11/22/18 - Bug 28962560: jznuPrintEJson
   dmcmahon    11/19/18 - Add jznEventQueueOsonDetect for eJson support
   dmcmahon    11/11/18 - Bug 28914360: jznuComponentToOraDate
   dmcmahon    11/02/18 - Bug 28875973: __VA_ARGS__ macro needs NULLs
   sriksure    09/27/18 - Bug 28717127: Rationalize includes in public jzn*.h
   dmcmahon    09/13/18 - Bug 28645808: prototype for JsonEventWriterCreateVA
   dmcmahon    09/05/18 - bug 28603908: jznuToString, jznuScalarToString
   sriksure    08/23/18 - Bug 28546488: XDK C JSON APIs enhancements
   dmcmahon    06/26/18 - bug 28249640: JznEventSourceConvertEvent
   dmcmahon    06/22/18 - bug 28215551: add JZN_BULK_LOAD
   zliu        04/20/18 - Bug 27896458: add JZN_NEW_PATH_CONSTRUCT
   zliu        04/20/18 - Bug 27896458: add JZN_PATH_NO_EXPAND_PRED_WITH_ARRAY
   dmcmahon    03/06/18 - bug 27624933: BSON eJSON event source
   zliu        02/05/18 - Bug 27502186: add jznuGetPrintCtxFlags(jznuPrint()
   dmcmahon    01/22/18 - Add jznuPrintFlushIfNecessary()
   dmcmahon    01/17/18 - Add jznuPrintSetBufStream()
   dmcmahon    10/29/17 - Bug 27039375: jznEventQueue
   morgiyan    07/15/17 - Bug 26439848: Add JSON encoding name 
                          (JZN_UTF8_ENC, JZN_UTF16BE_ENC, JZN_UTF16LE_ENC,
                           JZN_UTF32LE_ENC, JZN_UTF32BE_ENC).
   dmcmahon    07/13/17 - bug 26432363: JZN_ALLOW_LEADING_BOM
   dmcmahon    06/15/17 - Bug 26284044: jznuPrintGetOutputLen return size_t
   sriksure    06/05/17 - Bug 26199675: Add JZN_INPUT_UTF8
   dmcmahon    05/02/17 - bug 25987946: jznuOraDateToComponent()
   zliu        03/23/17 - JZN_STANDALONE_FUNC_CALL, standalone func call
   zliu        03/13/17 - Bug 25722364, 25722384:
                          sql/json path expr typecheck at compile time
                          add JZN_TYP_CHK, JZN_STRICT_TYP_CMP
   dmcmahon    02/08/17 - Bug 25520497: added jznuPrintIsUnicode
   dmcmahon    12/22/16 - Bug 25311444: added jznuPrintIsClob
   dmcmahon    11/30/16 - Bug 25178101: jznuPrintKeyQueued and
                          jznuPrintConcatUnicode
   dmcmahon    11/18/16 - Bug 25121773: expose jznuOraDateToString
   dmcmahon    11/09/16 - Bug 25072880: add jznEvent[Source/Writer]CreateVA()
   dmcmahon    09/19/16 - Bug 24691066: deprecate JZN_ALLOW_RUBOUT
   dmcmahon    08/20/16 - Add JZN_CONVERT, JZN_LOSSY_CHECK
   dmcmahon    05/07/16 - Bug 23246515 - enhance serializer
   prthiaga    02/10/15 - Bug 22242390: Expose jznuFormatNumString &
                          jznuOraNumToString
   dmcmahon    11/10/15 - Bug 22180304 - Add JZN_ALLOW_RUBOUT
   dmcmahon    10/21/15 - Bug 22082373 - add JZN_MEMCTX
   prthiaga    09/09/15 - Bug 21812442: Expose jznuFormatDouble
   dmcmahon    07/21/15 - Bug 21474255 - jznuToString() documentation
   dmcmahon    07/09/15 - Move jznuFireFatalError to jznerr.h
   dmcmahon    05/27/15 - Bug 21151692 - NCHAR and UTF8 output
   dmcmahon    05/07/15 - Bug 21052018 - add jznuPrintSetWideMode()
   dmcmahon    04/30/15 - Bug 20992659 - JZNU_PRINT_NLS_CHARSET
   dmcmahon    03/12/15 - Creation

*/

#ifndef JZNEV_ORACLE
# define JZNEV_ORACLE

#ifndef ORATYPES
# include <oratypes.h> 
#endif

#ifndef ORASTRUC
# include <orastruc.h>
#endif

#ifndef XML_ORACLE
# include <xml.h>
#endif

#ifndef JZNTYPES_ORACLE
# include <jzntypes.h>
#endif

/*
** Public Types and Functions for JSON Event Source
*/
#define JsonEventSource                   jznEventSource

/*
** Forward Declarations
*/
typedef struct jznEventSource jznEventSource;


/*
** Public Functions for JSON Event Source
** NOTES:
**   Supported varargs parameters:
**   ===================================
**   Property Name        Expected Type
**   ===================================
**   JZN_FORMAT           oratext * ("JSON", "OSON", "BSON")
**   JZN_ALLOW            ub4
**   JZN_OSON_CONFIG_SIZE ub4
**   JZN_DUPCHECK         boolean
**   JZN_LOSSY_CHECK      boolean
**   JZN_VALIDATE         boolean
**   JZN_ENDPAIR          boolean
**   JZN_BULK_LOAD        boolean
**   JZN_FILE             oratext *
**   JZN_BUFFER           void *
**   JZN_BUFFERLEN        ub4
**   JZN_STREAM           orastream *
**   JZN_MEMCTX           oramemctx *
**   JZN_FATAL_HANDLER    void (*jznFatalErrHandler)(xmlctx *, oratext *)
**   JZN_MODE             ub4
**   JZN_BLKSIZE          ub4
*/

/*
** Construct a JSON event source with variable parameter list.
** Returns NULL on an error, either bad parameters or out of memory.
** The <jerr> variable will contain a JZNERR error code if construction fails.
*/
JsonEventSource *JsonEventSourceCreate (xmlctx *xctx, jsonerr *jerr, ...);

JsonEventSource *JsonEventSourceCreateVA(xmlctx *xctx, jsonerr *jerr,
                                         va_list va);

#define JsonEventSourceReset              JznEventSourceReset
#define JsonEventSourceDestroy            JznEventSourceDestroy
#define JsonEventSourceSetEncodingMode    JznEventSourceSetEncodingMode
#define JsonEventSourceGetErrorMessage    JznEventSourceGetErrorMessage
#define JsonEventSourceSetInput           JznEventSourceSetInput
#define JsonEventSourceNextEvent          JznEventSourceNextEvent
#define JsonEventSourceSkipEvent          JznEventSourceSkipEvent
#define JsonEventSourceSkipObject         JznEventSourceSkipObject
#define JsonEventSourceValidateOnly       JznEventSourceValidateOnly
#define JsonEventSourceValidateMode       JznEventSourceValidateMode
#define JsonEventSourceSetFieldList       JznEventSourceSetFieldList
#define JsonEventSourceConvertEvent       JznEventSourceConvertEvent
#define JsonEventSourceValidate           JznEventSourceValidate


/*
** Public constants for JSON Event record
*/
#define JsonEventRecord         jznEventRecord
#define JsonEventKey            JznEventKey
#define JsonEventKeyLen         JznEventKeyLen
#define JsonEventValueType      JznEventValueType
#define JsonEventValue          JznEventValue
#define JsonEventValueLen       JznEventValueLen
#define JsonEventIntValue       JznEventIntValue
#define JsonEventLongValue      JznEventLongValue
#define JsonEventUintValue      JznEventUintValue
#define JsonEventUlongValue     JznEventUlongValue
#define JsonEventShortValue     JznEventShortValue
#define JsonEventUshorValue     JznEventUshortValue
#define JsonEventFloatValue     JznEventFloatValue
#define JsonEventDoubleValue    JznEventDoubleValue
#define JsonEventTimeValue      JznEventTimeValue
#define JsonEventPosition       JznEventPosition

/*
** Public Types for JSON Event Writer
*/
#define JsonEventWriter         jznEventWriter

/*
** Forward Declaration
*/
typedef struct jznEventWriter jznEventWriter;


/*
** Public Functions for JSON Event Writer
** NOTES:
**   Supported varargs parameters:
**   ===================================
**   Property Name       Expected Type
**   ===================================
**   JZN_PRETTY          boolean
**   JZN_ASCII           boolean
**   JZN_EXTENSIBLE_BUF  boolean
*/

/*
** Construct a JSON event writer with variable parameter list.
** Returns NULL on an error, generally for an out of memory condition.
** The <jerr> variable will contain a JZNERR error code if construction fails.
*/
JsonEventWriter *JsonEventWriterCreate(xmlctx *xctx, jsonerr *jerr, ...);

JsonEventWriter *JsonEventWriterCreateVA(xmlctx *xctx, jsonerr *jerr,
                                         va_list va);

#define JsonEventWriterReset              JznEventWriterReset
#define JsonEventWriterDestroy            JznEventWriterDestroy
#define JsonEventWriterSetEncodingMode    JznEventWriterSetEncodingMode
#define JsonEventWriterSetFlags           JznEventWriterSetFlags
#define JsonEventWriterGetErrorMessage    JznEventWriterGetErrorMessage
#define JsonEventWriterSetBuffer          JznEventWriterSetBuffer
#define JsonEventWriterSetStream          JznEventWriterSetStream
#define JsonEventWriterPutEvent           JznEventWriterPutEvent
#define JsonEventWriterPutString          JznEventWriterPutString
#define JsonEventWriterFlush              JznEventWriterFlush
#define JsonEventWriterGetBuffer          JznEventWriterGetBuffer
#define JsonEventWriterPushEvent          JznEventWriterPushEvent 
#define JsonEventWriterStartObject        JznEventWriterStartObject
#define JsonEventWriterEndObject          JznEventWriterEndObject
#define JsonEventWriterStartArray         JznEventWriterStartArray
#define JsonEventWriterEndArray           JznEventWriterEndArray
#define JsonEventWriteKey                 JznEventWriteKey
#define JsonEventWriteString              JznEventWriteString
#define JsonEventWriteBinary              JznEventWriteBinary
#define JsonEventWriteInteger             JznEventWriteInteger
#define JsonEventWriteUnsignedInteger     JznEventWriteUnsignedInteger
#define JsonEventWriteLong                JznEventWriteLong
#define JsonEventWriteUnsignedLong        JznEventWriteUnsignedLong
#define JsonEventWriteFloat               JznEventWriteFloat
#define JsonEventWriteDouble              JznEventWriteDouble
#define JsonEventWriteTrue                JznEventWriteTrue
#define JsonEventWriteFalse               JznEventWriteFalse
#define JsonEventWriteBoolean             JznEventWriteBoolean
#define JsonEventWriteNull                JznEventWriteNull
#define JsonEventWriteNumber              JznEventWriteNumber
#define JsonEventWriteDate                JznEventWriteDate
#define JsonEventWriteTimestamp           JznEventWriteTimestamp
#define JsonEventWriteStream              JznEventWriteStream
#define JsonEventWriteOCIString           JznEventWriteOCIString
#define JsonEventWriteOCIRaw              JznEventWriteOCIRaw
#define JsonEventWriteOCINumber           JznEventWriteOCINumber
#define JsonEventWriteOCIDate             JznEventWriteOCIDate
#define JsonEventWriteOCIDateTime         JznEventWriteOCIDateTime

/*
** Argument names for varargs create functions
*/
#define JZN_ENCODING         (oratext *)"encoding"
#define JZN_CONVERT          (oratext *)"convert_input"
#define JZN_LOSSY_CHECK      (oratext *)"lossy_check"
#define JZN_BUFFER           (oratext *)"buffer"
#define JZN_BUFFERLEN        (oratext *)"buffer_length"
#define JZN_STREAM           (oratext *)"stream"
#define JZN_FILE             (oratext *)"file"
#define JZN_PARSEMODE        (oratext *)"mode"
#define JZN_FORMAT           (oratext *)"format"
#define JZN_SCHEMA           (oratext *)"schema"
#define JZN_ALLOW            (oratext *)"allow"
#define JZN_BSONTYPES        (oratext *)"bson_types"
#define JZN_BSON_EJSON       (oratext *)"ejson_patterns"
#define JZN_OSON_EJSON       (oratext *)"oson_ejson"
#define JZN_FIREBASE_TYPES   (oratext *)"firebase_patterns"
#define JZN_DUPCHECK         (oratext *)"duplicate_check"
#define JZN_VALIDATE         (oratext *)"validate_only"
#define JZN_BLKSIZE          (oratext *)"blocksize"
#define JZN_ENDPAIR          (oratext *)"endpair"
#define JZN_PRETTY           (oratext *)"pretty_print"
#define JZN_ASCII            (oratext *)"ascii_print"
#define JZN_FATAL_HANDLER    (oratext *)"fatal_handler"
#define JZN_MEMCTX           (oratext *)"memory_context"
#define JZN_TYP_CHK          (oratext *)"type_check"
#define JZN_BULK_LOAD        (oratext *)"bulk_load"
#define JZN_EXTENSIBLE_BUF   (oratext *)"extensible_buf"
#define JZN_MODE             (oratext *)"create_mode"
#define JZN_OSON_CONFIG_SIZE (oratext *)"oson_config_size"
#define JZN_PRINT_ERRORS     (oratext *)"print_errors"
#define JZN_PRINT_FLAGS      (oratext *)"print_flags"
#define JZN_DOM              (oratext *)"dom"
#define JZN_CSID             (oratext *)"csid"

#define JZN_ORAMEMCTX        JZN_MEMCTX /* ### Obsolete ### */

/* doing static type check for sql/json path language */

#define JZN_STRICT_TYP_CMP        (oratext *)"strict_typ_cmp"
#define JZN_STANDALONE_FUNC_CALL  (oratext *)"standalone_fun_call"
/* 
 * -----------------------------------------
 * strict type comparison semantics:
 *
 * $.a?(@.b == "34")  
 *   is compiled into 
 * $.a?(@.b.stringOnly() == "34")  
 *
 * $.a?(@.b == true)  
 *   is compiled into 
 * $.a?(@.b.booleanOnly() == true)  
 *
 * $.a?(@.b == 34)  
 *   is compiled into 
 * $.a?(@.b.numberOnly() == 34)  
 *
 * -----------------------------------------
 * lax type comparsion semantics:
 *
 * $.a?(@.b == "34")  
 *   is compiled into 
 * $.a?(@.b.string() == "34")  
 *
 * It matches both {"a"{"b":"34"}}  and {"a"{"b":34}}
 *
 * $.a?(@.b == true)  
 *   is compiled into 
 * $.a?(@.b.boolea() == true)  
 *
 * It matches both {"a"{"b":true}}  and {"a"{"b":"true"}}
 *
 * $.a?(@.b == 34)  
 *   is compiled into 
 * $.a?(@.b.number() == 34)  
 *
 * It matches both {"a"{"b":34}}  and {"a"{"b":"34"}}
 */

/* do not add array[*] in front of predicate check in jznpAddArrayStepForPred()
 * called by jznpCheckPredicate()
 * Such semantics is only correct for json_exists(), but not other cases
 */
#define JZN_PATH_NO_EXPAND_PRED_WITH_ARRAY (oratext *)"no_exp_pred_with_array"
/* support dotdot path and step after predicate and multi-pred in path
 * construct, we need to add this temporarily so that c xdk label can be
 * picked up by RDBMS without introducing diffs
 */
#define JZN_NEW_PATH_CONSTRUCT (oratext *)"new_path_construct"
/*
 * Request one of several array subscript semantics:
 *   "STRICT"    - steps must be ordered and not overlap
 *   "MERGED"    - de-duplicate and reorder
 *   "PRESERVE"  - duplicates/overlaps allowed, preserve existing order
 */
#define JZN_PATH_ARRAY_MODE (oratext *)"array_subscripts"
/*
 * allow calculations to parse
 */
#define JZN_ALLOW_CALC_PATHS (oratext *)"allow_calcs"
/*
 * allow paths with leading '@' signs (e.g. within NESTED_PATH)
 */
#define JZN_ALLOW_RELATIVE_CTX (oratext *)"allow_relative_paths"
/*
 * allow path-to-path in predicates: $.a?(@.b > @.c)
 */
#define JZN_ALLOW_PATH_TO_PATH (oratext *)"allow_path_to_path"
/*
 * allow bind-variable-rooted paths: $tmp.a.b[*].c
 */
#define JZN_ALLOW_BIND_PATHS (oratext *)"bind_based_paths"

/*
** Non-standard behavior flags
*/

#define JZN_ALLOW_UNQUOTED_NAMES      0x00000001
#define JZN_ALLOW_SINGLE_QUOTES       0x00000002
#define JZN_ALLOW_UNQUOTED_UNICODE    0x00000004
#define JZN_ALLOW_NUMERIC_QUIRKS      0x00000008
#define JZN_ALLOW_CONTROLS_WHITESPACE 0x00000010
#define JZN_ALLOW_TRAILING_COMMAS     0x00000020
#define JZN_ALLOW_MIXEDCASE_KEYWORDS  0x00000040
#define JZN_ALLOW_SCALAR_DOCUMENTS    0x00000080
#define JZN_ALLOW_RUBOUT              0x00000100 /* deprecated */
#define JZN_ALLOW_LEADING_BOM         0x00000200
#define JZN_ALLOW_COMMENTS            0x00000400

/* Flags from JSON5 and ECMA Script 5.1 */
#define JZN_ALLOW_ECMA_ESCAPES        0x00000800 /* \v \a \e \0 \' */
#define JZN_ALLOW_TABS_IN_STRINGS     0x00001000 /* \t \v \f allowed */
#define JZN_ALLOW_UNICODE_WHITESPACE  0x00002000 /* U+00A0 U+2028 U+2029 */
#define JZN_ALLOW_HEX_NUMBERS         0x00004000
#define JZN_ALLOW_CONTINUATIONS       0x00008000

#define JZN_ALLOW_NONE                0x00000000
#define JZN_ALLOW_ALL                 0x0000FFFF

/*
** Input character-encoding modes
*/

#define JZN_INPUT_DETECT       0  /* Auto-detect Unicode input     */
#define JZN_INPUT_DIRECT       1  /* Input is UTF-8                */
#define JZN_INPUT_CONVERT      2  /* Input requires CS conversion  */
#define JZN_INPUT_UTF16        3  /* Input is UCS2 or UTF-16       */
#define JZN_INPUT_UTF16_SWAP   4  /* Input requires byte swapping  */
#define JZN_INPUT_LATIN        5  /* Input is Latin-1 (castable)   */
#define JZN_INPUT_WINDOWS      6  /* Input is Windows-1252 (table) */
#define JZN_INPUT_UTF8         JZN_INPUT_DIRECT /* Alias for UTF-8 */

/*
** JSON Unicode encoding names
*/

#define JZN_ENC_UNKNOWN        0 
#define JZN_ENC_UTF8           1 /* UTF-8 */
#define JZN_ENC_UTF16BE        2 /* UTF-16BE */
#define JZN_ENC_UTF16LE        3 /* UTF-16LE */
#define JZN_ENC_UTF32BE        4 /* UTF-32BE */
#define JZN_ENC_UTF32LE        5 /* UTF-32LE */

/*
** JSON EVENT SOURCE INTERFACE
**
** The JSON parser and BSON decoder both implement this interface.
** This allows consumers to run independently of the type of input.
**
** ### AVRO and ProtoBuf are still to-do.
*/

/*
** Event record
*/
typedef struct jznEventRecord
{
  jznevent    evtype;    /* Event code */
  jznvaltype  valtype;   /* Value type (if applicable) */
  jzn_sval_union var;
} jznEventRecord;

#define JznEventKey(rec)         ((rec).var.nam.name)
#define JznEventKeyLen(rec)      ((rec).var.nam.namelen)
#define JznEventValueType(rec)   ((rec).valtype)
#define JznEventValue(rec)       ((rec).var.val.value)
#define JznEventValueLen(rec)    ((rec).var.val.valuelen)
#define JznEventIntValue(rec)    ((rec).var.ival.intval)
#define JznEventLongValue(rec)   ((rec).var.ival.longval)
#define JznEventShortValue(rec)  ((rec).var.ival.shortval)
#define JznEventUintValue(rec)   ((rec).var.uval.intval)
#define JznEventUlongValue(rec)  ((rec).var.uval.longval)
#define JznEventUshortValue(rec) ((rec).var.uval.shortval)
#define JznEventFloatValue(rec)  ((rec).var.fval.ftval)
#define JznEventDoubleValue(rec) ((rec).var.dval.fpval)
#define JznEventTimeValue(rec)   ((rec).var.tval.dtval)
#define JznEventPosition(rec)    ((rec).var.cont.position)

/*
** JSON event stream interface
**
**   reset           - clear context for next input
**   destroy         - destroy context and free memory
**   getErrorMessage - return a message string
**   setEncodingMode - for textual JSON, set input encoding mode
**   setInput        - set next input buffer/stream/file
**   getNextEvent    - fill record with next event info, returning error code
*/
struct jznEventSource
{
  void       *ctx;
  void      (*reset)(void *ctx);
  void      (*destroy)(void *ctx);
  oratext  *(*getErrorMessage)(void *ctx);
  void      (*setEncodingMode)(void *ctx, ub2 csmode);
  sword     (*setInput)(void *ctx, uword inptype, void *inp, ub4 inplen);
  sword     (*getNextEvent)(void *ctx, jznEventRecord *rec);
  void      (*skipEvent)(void *ctx);
  void      (*skipObject)(void *ctx);
  boolean   (*validateOnly)(void *ctx, boolean mode);
  sword     (*validate)(void *ctx);
  void      (*setFieldList)(void *ctx, ub4 nfields, 
                            oratext **names, ub4 *name_lens);
  sword     (*convertEvent)(void *ctx, jznEventRecord *rec, sb2 csid);
};

/*
** Integer codes for event source input
*/
#define JZN_EVENT_SOURCE_INPUT_BUFFER    0
#define JZN_EVENT_SOURCE_INPUT_STREAM    1
#define JZN_EVENT_SOURCE_INPUT_FILE      2

/*
** Reset the event source for another parse/decode operation
*/
#define JznEventSourceReset(jsrc) \
  (*((jsrc)->reset))((jsrc)->ctx)

/*
** Destroy an event source
*/
#define JznEventSourceDestroy(jsrc) \
  (*((jsrc)->destroy))((jsrc)->ctx)

/*
** Change the encoding mode of an event source
*/
#define JznEventSourceSetEncodingMode(jsrc, csmode) \
  (*((jsrc)->setEncodingMode))((jsrc)->ctx, (csmode))

/*
** Returns an error message string
*/
#define JznEventSourceGetErrorMessage(jsrc) \
  (*((jsrc)->getErrorMessage))((jsrc)->ctx)

/*
** Change the input to a new source which can be a buffer, stream, or file.
**
**   Buffer   inp    - points to the buffer contents
**            inplen - ub4 buffer length
**   Stream   inp    - points to an orastream
**            inplen - unused
**   File     inp    - points to file name string
**            inplen - unused
**
** Returns an error code on failure.
*/
#define JznEventSourceSetInput(jsrc, inptype, inp, inplen) \
  (*((jsrc)->setInput))((jsrc)->ctx, (inptype), (inp), (inplen))

/*
** Get the next event from the source
**
** The caller's jznEventRecord is filled with the details.
** Note that pointer values in the record (e.g. to the member name
** or scalar value) are stable only as long as no further events
** are consumed, as they may point to internal memory structures of
** the event source.
**
** Returns an error code on failure.
*/
#define JznEventSourceNextEvent(jsrc, rec) \
  (*((jsrc)->getNextEvent))((jsrc)->ctx, (rec))

/*
** Request that the event source skip the next value.
** This should only be called on a JZNEV_KEY event (start pair);
** otherwise, the request is ignored. Note that some sources
** may not be able to honor the request. If the request can be
** honored, the member of the pair is skipped and a JZNEV_ITEM
** is delivered with JZNVAL_UNKNOWN as the value type.
** 
*/
#define JznEventSourceSkipEvent(jsrc) \
  (*((jsrc)->skipEvent))((jsrc)->ctx)

/*
** Request that the event source skip to the end of the current
** object (structure or array).
** This should only be called when expecting another JZNEV_KEY
** event (structure) or JZNEV_ITEM (array), otherwise, the request
** is ignored. Note that some sources may not be able to honor the
** request. If the request can be honored, the event source skips
** to the end of the current object and delivers the appropriate
** ending event.
** 
*/
#define JznEventSourceSkipObject(jsrc) \
  (*((jsrc)->skipObject))((jsrc)->ctx)

/*
** Set the validation mode for the event source on or off.
** Returns the previous setting (TRUE/FALSE).
*/
#define JznEventSourceValidateMode(jsrc, mflg) \
  (*((jsrc)->validateOnly))((jsrc)->ctx, mflg)

/*
** Request that the event source run in validation mode
** If honored, member names and values will be yielded
** as empty strings. The intent is to allow the caller to
** quickly check that the source is syntactically or
** structurally correct without needing full events.
*/
#define JznEventSourceValidateOnly(jsrc) \
  (*((jsrc)->validateOnly))((jsrc)->ctx, TRUE)

/*
** Set a list of desired field names (leaf names, not paths).
** Other fields may be internally skipped by the event source.
*/
#define JznEventSourceSetFieldList(jsrc, nfields, names, name_lens) \
  (*((jsrc)->setFieldList))((jsrc)->ctx, (nfields), (names), (name_lens))

/*
** Convert event from the source
**
** The jznEventRecord is converted to the target character set ID.
** A 0 for csid indicates a conversion to the "native" character set.
** Conversion is only done for JZNVAL_STRING item events, or for
** field name events.
**
** Returns an error code on failure. Sources that cannot implement
** conversion will return JZNERR_UNIMPLEMENTED.
*/
#define JznEventSourceConvertEvent(jsrc, rec, csid)  \
  (*((jsrc)->convertEvent))((jsrc)->ctx, (rec), (csid))

/*
** Read the event source to completion,
** returning any error code encountered.
** NOTE: One could implement the 'validate' callback of the jznEventSource
**       API to override the default behavior.
*/
sword JznEventSourceValidate(jznEventSource *jsrc);

/*
** Create an event source based on variable parameters.
**
**   JZN_FORMAT    :   oratext *
**                     Valid values: "JSON", "BSON"
**   JZN_ALLOW     :   ub4
**                     Bit flags (JSON parser only)
**   JZN_DUPCHECK  :   boolean
**                     TRUE = disallow duplicates (JSON parser only)
**   JZN_BSONTYPES :   boolean
**                     TRUE = native BSON data types as output
**
** ### These aren't supported yet:
**
**   JZN_SCHEMA    :   void *
**                     Schema pointer (e.g. AVRO, Protobuf)
**   JZN_ENCODING  :   oratext *
**                     Encoding character set (name)
**
** Returns a NULL if the creation fails, either for out of memory or
** bad parameters.
**
** ### This is an outdated interface;
** ### JsonEventSourceCreate() can return an error code.
** ### However, this interface supports some internal parameters that
** ### haven't been made public. Make sure your arguments are correct for
** ### internal uses, ensuring that a NULL return always means out of memory.
*/
jznEventSource *JznEventSourceCreate(xmlctx *xctx, ...);

jznEventSource *JznEventSourceCreateVA(xmlctx *xctx, va_list va);

/*
** JSON EVENT WRITER INTERFACE
**
** The JSON parser and BSON decoder both implement this interface.
** This allows event writers to operate independently of the type of output.
**
** ### AVRO and ProtoBuf are still to-do.
*/

/*
** JSON event writer interface
**
**   reset           - clear context for next output
**   destroy         - destroy context and free memory
**   getErrorMessage - return a message string
**   setOutput       - set next input buffer/stream
**   writeEvent      - output the next event
**   flush           - flush buffered output to stream
*/
struct jznEventWriter
{
  void       *ctx;
  void      (*reset)(void *ctx);
  void      (*destroy)(void *ctx);
  oratext  *(*getErrorMessage)(void *ctx);
  void      (*setEncodingMode)(void *ctx);
  void      (*setFlags)(void *ctx, ub4 flags);
  sword     (*setOutputBuffer)(void *ctx, void *buf, ub4 buflen);
  sword     (*setOutputStream)(void *ctx, orastream *out);
  sword     (*writeEvent)(void *ctx, jznEventRecord *rec);
  sword     (*writeString)(void *ctx, oratext *str, ub4 slen);
  sword     (*flush)(void *ctx);
  sword     (*getBuffer)(void *ctx, void **buf, ub4 *len);
  sword     (*pushEvent)(void *ctx, jznevent evtyp, jznvaltype vtyp, ...);
};

/*
** Reset the event writer for another operation
*/
#define JznEventWriterReset(jw) \
  (*((jw)->reset))((jw)->ctx)

/*
** Destroy an event writer
*/
#define JznEventWriterDestroy(jw) \
  (*((jw)->destroy))((jw)->ctx)

/*
** Set writer encoding mode to encode to default character set
** (ignored for BSON writers)
*/
#define JznEventWriterSetEncodingMode(jw) \
  (*((jw)->setEncodingMode))((jw)->ctx)

/*
** Set writer serialization flags (e.g. ASCII, PRETTY, BSON_PATTERNS)
*/
#define JznEventWriterSetFlags(jw, fl) \
  (*((jw)->setFlags))((jw)->ctx, (fl))


/*
** Returns an error message string
*/
#define JznEventWriterGetErrorMessage(jw) \
  (*((jw)->getErrorMessage))((jw)->ctx)

/*
** Set an output buffer for the writer
*/
#define JznEventWriterSetBuffer(jw, bf, bl) \
  (*((jw)->setOutputBuffer))((jw)->ctx, (bf), (bl))

/*
** Set an output buffer for the writer
*/
#define JznEventWriterSetStream(jw, os) \
  (*((jw)->setOutputStream))((jw)->ctx, (os))

/*
** Write an event record
*/
#define JznEventWriterPutEvent(jw, r) \
  (*((jw)->writeEvent))((jw)->ctx, (r))

/*
** Write an event record (atomised version)
*/

/* This is actually a concrete function, not abstract, but it was added as
** a function pointer to avoid exporting a new function.
*/
#define JznEventWriterPushEvent(jw, evtyp, vtyp, ...) \
  (*((jw)->pushEvent))((jw)->ctx, (evtyp), (vtyp), __VA_ARGS__)

/*
** Write Start Object Event
*/
#define JznEventWriterStartObject(jw) \
  (JznEventWriterPushEvent((jw), (JZNEV_START_OBJECT), (JZNVAL_NONE), NULL))

/*
** Write End Object Event
*/
#define JznEventWriterEndObject(jw) \
  (JznEventWriterPushEvent((jw), (JZNEV_END_OBJECT), (JZNVAL_NONE), NULL))

/*
** Write Start Array Event
*/
#define JznEventWriterStartArray(jw) \
  (JznEventWriterPushEvent((jw), (JZNEV_START_ARRAY), (JZNVAL_NONE), NULL))

/*
** Write End Array Event
*/
#define JznEventWriterEndArray(jw) \
  (JznEventWriterPushEvent((jw), (JZNEV_END_ARRAY), (JZNVAL_NONE), NULL))

/*
** Write Key Event
*/
#define JznEventWriteKey(jw, sval, slen) \
  (JznEventWriterPushEvent((jw), (JZNEV_KEY), (JZNVAL_NONE), \
                           (sval), (slen)))

/*
** Write Event - String
*/
#define JznEventWriteString(jw, sval, slen) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_STRING), \
                           (sval), (slen)))

/*
** Write Event - Binary
*/
#define JznEventWriteBinary(jw, bval, blen) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_BINARY), \
                           (bval), (blen)))

/*
** Write Event - Vector
*/
#define JznEventWriteVector(jw, bval, blen) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_VECTOR), \
                           (bval), (blen)))

/*
** Write Event - Integer
*/
#define JznEventWriteInteger(jw, ival) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_SINT), \
                           (ival)))

/*
** Write Event - Unsigned Integer
*/
#define JznEventWriteUnsignedInteger(jw, ival) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_UINT), \
                           (ival)))

/*
** Write Event - Long
*/
#define JznEventWriteLong(jw, lnval) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_SLONG), \
                           (lnval)))

/*
** Write Event - Unsigned Long
*/
#define JznEventWriteUnsignedLong(jw, lnval) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_ULONG), \
                           (lnval)))

/*
** Write Event - Float
*/
#define JznEventWriteFloat(jw, fval) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_FLOAT), \
                           (fval)))

/*
** Write Event - Double
*/
#define JznEventWriteDouble(jw, dval) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_DOUBLE), \
                           (dval)))

/*
** Write Event - TRUE
*/
#define JznEventWriteTrue(jw) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_TRUE), NULL))

/*
** Write Event - FALSE
*/
#define JznEventWriteFalse(jw) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_FALSE), NULL))

/*
** Write Event - Boolean
*/
#define JznEventWriteBoolean(jw, bval) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), \
                           ((bval) ? JZNVAL_TRUE : JZNVAL_FALSE), NULL))

/*
** Write Event - NULL
*/
#define JznEventWriteNull(jw) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_NULL), NULL))

/*
** Write Event - ORA Number
*/
#define JznEventWriteNumber(jw, nval, nlen) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_ORA_NUMBER), \
                           (nval), (nlen)))

/*
** Write Event - ORA Date
*/
#define JznEventWriteDate(jw, dval, dlen) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_ORA_DATE), \
                           (dval), (dlen)))

/*
** Write Event - ORA Timestamp
*/
#define JznEventWriteTimestamp(jw, tsval, tslen) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_ORA_TIMESTAMP), \
                           (tsval), (tslen)))

/*
** Write Event - Binary/Character Stream
*/
#define JznEventWriteStream(jw, strm, is_bin) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), \
    ((is_bin) ? JZNVAL_BIN_STREAM : JZNVAL_CHR_STREAM), (strm)))

/*
** Write Event - Vector Stream
*/
#define JznEventWriteVecStream(jw, strm) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), \
    (JZNVAL_VEC_STREAM), (strm)))

/*
** Write Event - OCIString
*/
#define JznEventWriteOCIString(jw, ostr) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_OCI_STRING), (ostr)))

/*
** Write Event - OCIRaw
*/
#define JznEventWriteOCIRaw(jw, oraw) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_OCI_RAW), (oraw)))

/*
** Write Event - OCINumber
*/
#define JznEventWriteOCINumber(jw, onum) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_OCI_NUMBER), (onum)))

/*
** Write Event - OCIDate
*/
#define JznEventWriteOCIDate(jw, odate) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_OCI_DATE), (odate)))

/*
** Write Event - OCIDateTime
*/
#define JznEventWriteOCIDateTime(jw, otime) \
  (JznEventWriterPushEvent((jw), (JZNEV_ITEM), (JZNVAL_OCI_DATETIME), (otime)))


/*
** Write a single string
** This API should be used instead of the event writer
** The output for BSON is questionable since a single string
** isn't a valid BSON.
*/
#define JznEventWriterPutString(jw, s, sl) \
  (*((jw)->writeString))((jw)->ctx, (s), (sl))

/*
** Flush an event writer
*/
#define JznEventWriterFlush(jw) \
  (*((jw)->flush))((jw)->ctx)

/*
** Get result buffer (for non-stream-based writers)
*/
#define JznEventWriterGetBuffer(jw, bf, bl) \
  (*((jw)->getBuffer))((jw)->ctx, (bf), (bl))

/*
** Create an event writer based on variable parameters.
**
**   JZN_FORMAT    :   oratext *
**                     Valid values: "JSON", "BSON"
**   JZN_PRETTY    :   boolean
**                     TRUE = pretty-print the output
**   JZN_ASCII     :   boolean
**                     TRUE = native ASCII (with escapes)
**   JZN_BSONTYPES :   boolean
**                     TRUE = special BSON data type mode
**
** ### These aren't supported yet:
**
**   JZN_SCHEMA    :   void *
**                     Schema pointer (e.g. AVRO, Protobuf)
**   JZN_ENCODING  :   oratext *
**                     Encoding character set (name)
**
** Returns a NULL if the creation fails, either for out of memory or
** bad parameters.
** ### This is an outdated interface;
** ### JsonEventWriterCreate() can return an error code.
** ### However, this interface supports some internal parameters that
** ### haven't been made public. Make sure your arguments are correct for
** ### internal uses, ensuring that a NULL return always means out of memory.
*/
jznEventWriter *JznEventWriterCreate(xmlctx *xctx, ...);

jznEventWriter *JznEventWriterCreateVA(xmlctx *xctx, va_list va);


/*
** JSON Oracle Binary encoder, decoder interfaces
**   - Facility for parsing and generating JSON Binary using event interfaces
*/

/*
** Context for parsing and generating JSON Binary data
*/
typedef struct JsonBinaryCtx  JsonBinaryCtx;

/*
** Callers must include <xti.h> to use XTI File System API
*/
struct xtiFSys;

/*
** NAME:
**   JsonBinaryCtxCreate
**
** PARAMETERS:
**   xctx    - IN: XML Context
**   memctx  - IN: Optional, an Ora Mem Context 
**   jerr    - OUT: Error code
**
** DESCRIPTION:
**   Creates the context for parsing and generating JSON binary data.
**
** NOTES:
**   - The XML context must have the error_handler and error_context
**     properties set.
**   - Caller must always check for return value of jerr before proceeding.
**   - It is strongly recommended to create an instance of JSON Binary Context
**     and reuse it for the scope of working environment, for all the
**     encoding and decoding calls for best performance.
*/
JsonBinaryCtx  *JsonBinaryCtxCreate(xmlctx  *xctx, oramemctx *memctx,
                                    jsonerr  *jerr);

/*
** NAME:
**   JsonBinaryCtxReset
**
** PARAMETERS:
**    bctx   - Json Binary context
**
** DESCRIPTION:
**    Resets the Json Binary context, clearing any associated state.
**
*/
void  JsonBinaryCtxReset(JsonBinaryCtx  *bctx);

/*
** NAME:
**   JsonBinaryCtxDestroy
**
** PARAMETERS:
**    bctx   - Json Binary context
**
** DESCRIPTION:
**   Destroys the Json Binary context.
**
*/
void  JsonBinaryCtxDestroy(JsonBinaryCtx  *bctx);

/*
** NAME:
**   JsonBinaryEncodeAll
**
** PARAMETERS:
**   bctx       - JSON Binary Context
**   jsrc       - JSON event source
**   outp       - Pointer to output to write to, can be oratext*, orastream*
**                or FILE*
**   bufsz      - Size of the output buffer, ignored for stream and file types
**   xfs        - XTI File system interface pointer (For file type)
**   out_typ    - Type of the output
**
** DESCRIPTION:
**   Read all the events emitted by the Event Source and encodes to Oracle
**   Binary format (OSON).
**
**
*/
jsonerr JsonBinaryEncodeAll(JsonBinaryCtx  *bctx, JsonEventSource *jsrc, 
                            void  *outp, ub4  *bufsz, struct xtiFSys *xfs,
                            uword  out_typ);

/*
** NAME:
**   JsonBinaryDecodeAll
**
** PARAMETERS:
**   bctx       - JSON Binary Context
**   jwctx      - JSON event writer
**   srcp       - Source handle to binary JSON data, can be ub1*, orastream*
**                or FILE*
**   size       - Size of the output buffer, ignored for stream and file types
**   xfs        - XTI File system interface pointer (For file type)
**   src_typ    - Type of the source
**
** DESCRIPTION:
**   Writes all the events using a Event Writer by parsing the Oracle Binary
**   format (OSON) input.
** 
*/
jsonerr JsonBinaryDecodeAll(JsonBinaryCtx *bctx, JsonEventWriter *jwctx,
                            void  *srcp, ub4  size,  struct xtiFSys *xfs,
                            uword  src_typ);


/*
** Integer codes for output types
*/
#define JZN_EVENT_OUTPUT_BUFFER    1
#define JZN_EVENT_OUTPUT_STREAM    2
#define JZN_EVENT_OUTPUT_FILE      3


/*
** JSON Serializer Interface
**
** This object is used to serialize: 
**   1. a series of JSON events 
**   2. a DOM tree. 
** into textual JSON output. 
** NOTE: Flags can be used to control the output.
*/

/*
** Pretty-printing adds newlines, spaces, and indention to the output
** ASCII forces Unicode characters to be rendered using escapes.
** TEST produces an event "listing" and not a JSON result.
**
** A typical use would be to set no flags and get a canonical output.
** If the target character set isn't Unicode, a valid escaped canonical
** can be produced by setting the ASCII-only flag.
*/
#define JZNU_PRINT_PRETTY            0x0001  /* pretty print, indention, etc */
#define JZNU_PRINT_ASCII             0x0002  /* escape non-ASCII characters  */
#define JZNU_PRINT_NUMFORMAT         0x0004  /* print numbers canonically    */
#define JZNU_PRINT_CANONICAL_DECIMAL 0x0008  /* print canonical digits       */
#define JZNU_PRINT_EXPONENTIAL       0x0100  /* Print double/float with 'E'  */
#define JZNU_PRINT_REPLACEMENT       0x0200  /* Replace bad byte codes       */
#define JZNU_PRINT_BASE64            0x0400  /* Use base-64 for binaries     */
#define JZNU_PRINT_DOUBLE            0x0800  /* Print Nan/Inf/-Inf natively  */
#define JZNU_PRINT_BADVALS           0x1000  /* Print bad/corrupt values     */
#define JZNU_PRINT_EJSON             0x2000  /* Print eJSON patterns         */
#define JZNU_PRINT_BSON_PATTERNS     0x4000  /* convert BSON patterns        */
#define JZNU_PRINT_TEST              0x8000  /* ### Hacky - remove this? ### */
/* Flags above 0x8000 reserved for internal use */

/*
** Context for the event printer
*/
typedef struct jznuPrintCtx jznuPrintCtx;

/*
** Create the print context 
** Returns a NULL on an out of memory condition.
*/
jznuPrintCtx *jznuPrintCreate(xmlctx *xctx, ub4 flags);
jznuPrintCtx *jznuPrintCreateEx(xmlctx *xctx, oramemctx *omctx, ub4 flags);

/*
** Reset print context's internal xmlctx
*/
void          jznuPrintSetXmlContext(jznuPrintCtx *pctx, xmlctx *xctx);

/* Get handle to the internal XML context for the print serializer */
xmlctx       *jznuPrintGetXmlContext(jznuPrintCtx *pctx);

/*
** Make a user sub-context for this print serializer of the specified size.
** Any old sub-context is freed. A pointer to the new sub-context is
** retained and is gettable in future. If sz is 0, the old sub-context is
** freed without creating a new one. Returns NULL on failure or when sz is 0.
*/
void *jznuPrintMakeUserContext(jznuPrintCtx *pctx, size_t sz);
/*
** Return a pointer to user sub-context, if any.
*/
void *jznuPrintGetUserContext(jznuPrintCtx *pctx);

/*
** Returns TRUE if the serializer is writing to a stream,
** FALSE if it's writing to an in-memory buffer.
*/
boolean jznuPrintIsStreaming(jznuPrintCtx *pctx);

/*
** Returns TRUE if the serializer is writing to a CLOB, FALSE otherwise.
**
** This is not the same as checking whether WIDE (UTF-16) output is
** being produced, since a CLOB can be in the DB character set in
** single-byte databases. Also, WIDE output may be requested for
** NVARCHAR/NCLOB cases.
*/
boolean jznuPrintIsClob(jznuPrintCtx *pctx);

/*
** Returns TRUE if the serializer is writing Unicode (i.e. UTF-8)
** via a conversion.
*/
boolean jznuPrintIsUnicode(jznuPrintCtx *pctx);

/*
** Indicates whether any errors were encountered and treated as warnings
** during the rendering. These are typically due to bad character data
** or character set conversions. The returned code is the most recent
** warning encountered, and may change if a later warning occurs. Once
** a warning code is set it is not cleared until a reset is done, so
** callers that don't need to see every warning can safely wait until
** the end of a serialization to get the last warning.
*/
sword   jznuPrintGetWarning(jznuPrintCtx *pctx);

/*
** Print serializer mode setters
**
** jznuSetPrettyMode - pretty-print the output
** jznuSetAsciiMode - \u escape non-ASCII characters
** jznuSetEJSONMode - print eJSON patterns
** jznuPrintSetTruncatingMode - truncate output to buffer length
** jznuPrintSetEncodingMode - convert output to DB character set
** jznuPrintSetOutputCharset - set the output character set;
**                             overrides SetEncodingMode
** jznuPrintSetNativeCharset - input to serializer is in DB character set;
**                             disables output conversion (unnecessary)
** jznuPrintSetWideMode - convert output to CLOB compatible format;
**    uses output conversion for single-byte character sets,
**    does UTF-16 conversion for multi-byte character sets.
** jznuPrintSetUnicodeOut - convert output to either UTF8 or UTF16 (for NCHAR)
*/
void jznuSetPrettyMode(jznuPrintCtx *pctx, boolean b);

void jznuSetAsciiMode(jznuPrintCtx *pctx, boolean b);

void jznuSetEJSONMode(jznuPrintCtx *pctx, boolean b);

void jznuPrintSetTruncatingMode(jznuPrintCtx *pctx, boolean truncate);

void jznuPrintSetEncodingMode(jznuPrintCtx *pctx, boolean doconvert);

void jznuPrintSetOutputCharset(jznuPrintCtx *pctx, sb2 csid);

void jznuPrintSetNativeCharset(jznuPrintCtx *pctx, boolean nativecs);

void jznuPrintSetWideMode(jznuPrintCtx *pctx, boolean clobflag);

void jznuPrintSetUnicodeOut(jznuPrintCtx *pctx, boolean uniflag);

/*
** Character set code paths in the print serializer:
**
**   If the input is not in the DB character set, e.g. from the
**   path engine, then jznuPrintSetNativeCharset(pctx, FALSE).
**
**   If the input is in the DB character set, e.g. from JSON aggregate
**   operator, then jznuPrintSetNativeCharset(pctx, TRUE).
**
**   If the output is in the DB character set, e.g. to VARCHAR2,
**   then jznuPrintSetEncodingMode(pctx, TRUE).
**   If the output is for CLOB, then jznuPrintSetWideMode(pctx, TRUE).
**   These are mutually exclusive calls.
**
**   If the output is to NVARCHAR, call jznuPrintSetUnicodeOut(pctx, TRUE).
**   If the output is to NCLOB or NVARCHAR using AL16UTF16, then also call
**   jznuPrintSetWideMode(pctx, TRUE).
*/

/*
** Destroy the print context 
*/
void jznuPrintDestroy(jznuPrintCtx *pctx);

/*
** NAME:
**   jznuPrintSetBuffer
** PARAMETERS:
**   pctx     (IN)  - serializer print context 
**   buffer   (IN)  - input buffer containing bytes to process
**   buflen   (IN)  - length of input buffer (in bytes)
** RETURNS:
**   status (JZNERR_OK if successful)
** DESCRIPTION:
**   set the buffer that the serializer will use as output 
*/
sword jznuPrintSetBuffer(jznuPrintCtx *pctx, oratext *buffer, ub4 buflen);

/*
** NAME:
**   jznuPrintSetStream
** PARAMETERS:
**   pctx     (IN)  - serializer print context 
**   stream   (IN)  - input stream
** RETURNS:
**   status (JZNERR_OK if successful)
** DESCRIPTION:
**   setup the stream that the serializer will use as output
** NOTE:
**   The stream must be opened externally before the first call to
**   jznPrintEvent.
*/
sword jznuPrintSetStream(jznuPrintCtx *pctx, orastream *stream);

/*
** NAME:
**   jznuPrintSetBufStream
** PARAMETERS:
**   pctx     (IN)  - serializer print context 
**   stream   (IN)  - input stream
**   buffer   (IN)  - input buffer containing bytes to process
**   buflen   (IN)  - length of input buffer (in bytes)
** RETURNS:
**   status (JZNERR_OK if successful)
** DESCRIPTION:
**   set the buffer that the serializer will use as output 
*/
sword jznuPrintSetBufStream(jznuPrintCtx *pctx,
                            oratext *buffer, ub4 buflen, orastream *stream);

/*
**
** NAME:
**   jznuGetPrintCtxFlags
**  PARAMETERS:
**   pctx     (IN)  - print context 
** RETURNS:
**   printerCtx flag ()
**  DESCRIPTION:
** Get current printerCtx serialization flags 
** (e.g. ASCII, PRETTY, BSON_PATTERNS)
*/
ub4 jznuGetPrintCtxFlags(jznuPrintCtx *pctx);

/*
** Change the print serialization flags.
** Do not call this after printing has started.
*/
void jznuSetPrintCtxFlags(jznuPrintCtx *pctx, ub4 flags);

/*
** NAME:
**   jznuPrintSetExtensible
** PARAMETERS:
**   pctx   (IN)  -   serializer print context 
** RETURNS:
**   status (JZNERR_OK if successful)
** DESCRIPTION:
**   setup the serializer to write to an extensible internal buffer.
** NOTE:
**   At the end, the buffer is available via jznuPrintGetOutput().
*/
sword jznuPrintSetExtensible(jznuPrintCtx *pctx);

/*
** Send an event to the print stream. For start-of-pair and end-of-pair
** events pass the name as the <str> argument. For item events pass the
** value as <str>. In all other cases <str> is ignored.
*/
ub2 jznuPrintEvent(jznuPrintCtx *pctx, jznevent ev,
                   ub1 *str, jznvaltype typ, ub4 len);


/*
** Send vector type as array of float/double to the print stream
** Error return code jsonerr, the vector library processing error code
** can be obtained from the call of jznuPrintGetVecErrStat()
*/
sword jznuPrintVectorEx(jznuPrintCtx *pctx, ub4 flags,
                        ub1 *vecbin, ub4 vecbinlen);
sword jznuPrintVector(jznuPrintCtx *pctx, ub1 *vecbin, ub4 vecbinlen);

#define JZNU_PRINT_VECTOR_EJSON  0x0001

/*
** Send an event to the print stream as an event record.
*/
ub2 jznuPrintRecord(jznuPrintCtx *pctx, jznEventRecord *rec);

/*
** Send an event to the print stream as an event record.
** If <is_unicode> is TRUE, the record is accepted in AL32UTF8
** regardless of the configured input character set of the printer.
** This allows callers to avoid lossy conversions through the
** input character set if the final output is Unicode.
*/
ub2 jznuPrintRecordDirect(jznuPrintCtx *pctx,
                          jznEventRecord *rec,
                          boolean is_unicode);
#define JZNU_ENABLE_DIRECT_EVENTS

/*
** Print a single scalar value and flush it to the output.
** Use this function instead of PrintEvent if this is the only
** output (e.g. JSON_VALUE).
*/
ub2 jznuPrintScalar(jznuPrintCtx *pctx, oratext *str, ub4 len);

/*
** Print an item using eJSON encoding based on the flags
*/
ub2 jznuPrintEJson(jznuPrintCtx *pctx, jznvaltype vtyp,
                   void *valptr, ub4 vallen);

/*
** Print enough messages to "close" the current rendering context
** with a valid JSON output.
*/
ub2 jznuPrintClose(jznuPrintCtx *pctx);

/*
** Send start/end object/array events to the print stream.
*/
#define jznuPrintStartObject(_p) \
  (jznuPrintEvent((_p), JZNEV_START_OBJECT, (ub1 *)0, JZNVAL_NONE, (ub4)0))

#define jznuPrintEndObject(_p) \
  (jznuPrintEvent((_p), JZNEV_END_OBJECT, (ub1 *)0, JZNVAL_NONE, (ub4)0))

#define jznuPrintStartArray(_p) \
  (jznuPrintEvent((_p), JZNEV_START_ARRAY, (ub1 *)0, JZNVAL_NONE, (ub4)0))

#define jznuPrintEndArray(_p) \
  (jznuPrintEvent((_p), JZNEV_END_ARRAY, (ub1 *)0, JZNVAL_NONE, (ub4)0))

/*
** Send a key (field name) event to the print stream.
*/
#define jznuPrintKey(_p, _key, _klen) \
  (jznuPrintEvent((_p), JZNEV_KEY, \
                  (void *)(_key), JZNVAL_NONE, (ub4)(_klen)))

/*
** Send a scalar value (in different forms) to the print stream
*/

#define jznuPrintNull(_p) \
  (jznuPrintEvent((_p), JZNEV_ITEM, (void *)"", JZNVAL_NULL, (ub4)0))

#define jznuPrintString(_p, _str, _slen) \
  (jznuPrintEvent((_p), JZNEV_ITEM, \
                  (void *)(_str), JZNVAL_STRING, (ub4)(_slen)))

#define jznuPrintOraNum(_p, _num, _nlen) \
  (jznuPrintEvent((_p), JZNEV_ITEM, \
                  (void *)(_num), JZNVAL_ORA_NUMBER, (ub4)(_nlen)))

#define jznuPrintOraDate(_p, _dat, _dlen) \
  (jznuPrintEvent((_p), JZNEV_ITEM, \
                  (void *)(_dat), JZNVAL_ORA_DATE, (ub4)(_dlen)))

#define jznuPrintOraTimestamp(_p, _dat, _dlen) \
  (jznuPrintEvent((_p), JZNEV_ITEM, \
                  (void *)(_dat), JZNVAL_ORA_TIMESTAMP, (ub4)(_dlen)))

/*
** Force out an item syntax element (a colon if this item follows
** a field key string, a comma if this item is the next item in an array).
** The item is presumed to be pre-rendered and the caller must deliver
** it via calls to jznuPrintAppendRaw, jznuPrintConcatJson. Alternatively,
** the item can be rendered in a piecewise fashion via jznuPrintItemPiece.
*/
#define jznuPrintItem(_p) \
  (jznuPrintEvent((_p), JZNEV_ITEM, (ub1 *)0, JZNVAL_UNKNOWN, (ub4)0))

/*
** Force out a previously queued key event, if any.
*/
#define jznuPrintPushKey(_p) \
  (jznuPrintEvent((_p), JZNEV_POS, (ub1 *)0, JZNVAL_UNKNOWN, (ub4)0))

/*
** Print a scalar string item with character set conversion.
** This outputs a JZNEV_ITEM event with a JZNVAL_STRING value type.
*/
ub2 jznuPrintStringItem(jznuPrintCtx *pctx, sb2 csid,
                        ub1 *value, ub4 len);

/*
** Queue a key (field name) event to the print stream.
** The memory pointed to by key must remain stable until flushed.
** Pass a NULL to clear the queued event.
*/
ub2 jznuPrintKeyQueued(jznuPrintCtx *pctx, oratext *key, ub4 klen);

/*
** Append pre-rendered raw bytes. Call as many times as
** necessary to flush an entire buffer or stream to the output.
** Note that the pre-rendered output bytes must have come
** from the same serializer (e.g. so that they are in the
** correct character set, or UCS2, as necessary). There is no
** requirement that the bytes be complete fragments; that is,
** this API is suitable for simply dumping the output from
** a stream of bytes.
*/
ub2 jznuPrintAppendRaw(jznuPrintCtx *pctx, void *buf, ub4 len);

/*
** Concatenate a pre-rendered JSON fragment. Unlike jznuPrintAppendRaw(),
** the input is presumed to be in the source character set, which may
** imply that a character-set conversion is needed. The buffer length
** is in bytes. On output, the buffer length is the number of bytes
** actually consumed from the input. If the source is a multi-byte
** character set, and the input length would result in splitting
** a multi-byte character sequence, the reader stops before consuming
** the partial character, leaving those bytes unconsumed in the buffer.
** The caller is then responsible for ensuring that they're prepended
** to subsequent fragments.
*/
ub2 jznuPrintConcatJson(jznuPrintCtx *pctx, oratext *buf, ub4 *len);

/*
** Concatenate a pre-rendered JSON fragment. Unlike jznuPrintConcatJson(),
** the input is presumed to be in UTF-8 or UCS2 and is auto-detected
** from the first block of data (the first block must pass at least 2 bytes
** from any data source longer than 1 byte). It is otherwise similar to
** jznuPrintConcatJson() in operation.
*/
ub2 jznuPrintConcatUnicode(jznuPrintCtx *pctx, boolean firstFlag,
                           ub1 *buf, ub4 *len);

/*
** jznuPrintItemPiece
**
** Description:
**   Render a scalar item in a piecewise fashion. Supported item types
**   are limited to JZNVAL_STRING and JZNVAL_BINARY.
** Arguments:
**   buf        - IN   Data bytes to output.
**   len        - IN   The number of bytes available in buf.
**                OUT  The number of bytes consumed from buf.
**   is_binary  - IN   TRUE if the data are raw binary bytes.
**                     Output will be rendered in hexadecimal.
**   start_flag - IN   Emit a double-quote before the data.
**   end_flag   - IN   Emit a double-quote after the data.
** Notes:
**   In binary mode, all bytes will be consumed on each call.
**   In text mode, a few bytes may remain unconsumed at the end
**   of the buffer if they would split a multi-byte character.
*/
ub2 jznuPrintItemPiece(jznuPrintCtx *pctx,
                       void *buf, ub4 *len, boolean is_binary,
                       boolean start_flag, boolean end_flag);

/*
** Flushed any buffered serializations to the output.
** No effect unless the output is a stream.
*/
ub2 jznuPrintFlush(jznuPrintCtx *pctx);

/*
** Flushed any buffered serializations to the output.
** A buffered stream won't be flushed if the initial
** buffer is still being filled.
*/
ub2 jznuPrintFlushIfNecessary(jznuPrintCtx *pctx);

/*
** Get a pointer to the output buffer.
*/
oratext *jznuPrintGetOutput(jznuPrintCtx *pctx);

/*
** Get the length of output.
**
** This allows the caller to get the length when the caller has
** requested the output go to a caller-supplied buffer. (The output
** may not be null-terminated.)
*/
size_t jznuPrintGetOutputLen(jznuPrintCtx *pctx);

/*
** Get a writer interface for a print serializer.
*/
jznEventWriter *jznuPrintToWriter(jznuPrintCtx *pctx);

/*
** Get an error message from the printer given an error code.
*/
oratext *jznuPrintGetError(jznuPrintCtx *pctx, sword errcode);

/*
** Specialized interface to allow printing items wrapped in an
** array. WrapStart will begin the array provisionally. At that
** point, both the array start and any subsequent writes will be
** buffered until either a WrapContinue or WrapEnd is signalled.
** If the caller is certain that the wrapper is needed, WrapContinue
** should be called; this clears the wrapping flag and flushes any
** buffered items. Further writes to the printer will then proceed
** normally. At the conclusion, WrapEnd should be called in lieu of
** closing the array. If a WrapContinue has been done, the array is
** closed normally. Otherwise, WrapEnd will remove the initial
** array open event, and then flush the data written to that point.
*/
ub2 jznuPrintWrapStart(jznuPrintCtx *pctx);
ub2 jznuPrintWrapContinue(jznuPrintCtx *pctx);
ub2 jznuPrintWrapEnd(jznuPrintCtx *pctx);

/*
 ** NAME:
 **  jznuToString
 **
 ** DESCRIPTION:
 **  Convert a value of 'jznvaltype' to its textual representation
 ** INPUTS:
 **   xmlctx     *xctx   : XML context
 **                        can be NULL unless type is JZNVAL_VECTOR
 **                        or if time zone adjustments are needed
 **   ub1        *val    : Buffer containting the value to be converted 
 **   ub4        *len    : Length of the 'val' buffer
 **   jznvaltype  vtype  : Type of the 'val'
 **   char       *outbuf : Buffer to hold the string form after the conversion
 **                        The string is null-terminated if enough space is
 **                        available.
 **   ub4        *outlen : On input, actual size in bytes of buffer 'outbuf'
 **                        On return, the number of bytes written to 'outbuf'
 **                        not including the trailing null.
 ** OUTPUTS:
 **   sword : indicates the status of the conversion.
 **           JZNERR_OK means conversion is successful.
 **           All callers should check the return status before proceeding.
 */
sword jznuToString(xmlctx *xctx, ub1 *val, ub4 len, jznvaltype vtype,
                   char *outbuf, ub4 *outlen);

/*
** NAME:
**  jznuScalarToString
**
** DESCRIPTION:
**   Convert a scalar value to its textual representation
**
** INPUTS:
**   xmlctx         *xctx   : XML context (NULL OK)
**   jznvaltype      vtype  : Scalar value type
**   jzn_sval_union *uptr   : Scalar value (union)
**   char           *outbuf : Buffer to hold the result
**   ub4            *outlen : IN/OUT length of the buffer 'outbuf'
**
** OUTPUTS:
**   sword : indicates the status of the conversion.
**           JZNERR_OK means conversion is successful.
**           All callers should check the return status before proceeding.
*/
sword jznuScalarToString(xmlctx *xctx,
                         jznvaltype vtype, jzn_sval_union *uptr,
                         char *outbuf, ub4 *outlen);

/*
** NAME:
**  jznuScalarEventToString
**
** DESCRIPTION:
**   Convert a scalar jznEventRecord to its textual representation
**
** INPUTS:
**   xmlctx         *xctx   : XML context (NULL OK)
**   jznEventRecord *rec    : Scalar value event to convert
**   char           *outbuf : Buffer to hold the result
**   ub4            *outlen : IN/OUT length of the buffer 'outbuf'
**
** OUTPUTS:
**   sword : indicates the status of the conversion.
**           JZNERR_OK means conversion is successful.
**           All callers should check the return status before proceeding.
*/
sword jznuScalarEventToString(xmlctx *xctx, jznEventRecord *rec,
                              char *outbuf, ub4 *outlen);

/*
** NAME:
**  jznuFormatDouble
**
** DESCRIPTION:
**  Render a double-precision floating-point value as a string.
**  The string buffer must be large enough (JZNU_NUM_DIGITS).
**  The output is null-terminated, and the length of the string is
**  returned.
**
** INPUTS:
**   double           dval          : Value to render
**   oratext         *outbuf        : Buffer to render into.
**
** OUTPUTS:
**   ub4                            : String length of output, 0 on error
*/
ub4 jznuFormatDouble(double dval, oratext *outbuf);

/*
** NAME:
**  jznuFormatFloat
**
** DESCRIPTION:
**  Render a single-precision floating-point value as a string.
**  The string buffer must be large enough (JZNU_NUM_DIGITS).
**  The output is null-terminated, and the length of the string is
**  returned.
**
** INPUTS:
**   float            fval          : Value to render
**   oratext         *outbuf        : Buffer to render into.
**
** OUTPUTS:
**   ub4                            : String length of output, 0 on error
*/
ub4 jznuFormatFloat(float fval, oratext *outbuf);

/*
** Double and Float formatting with flags. Flags:
**   JZNU_PRINT_EXPONENTIAL - Prints in E format with a radix.
*/
ub4 jznuFormatDoubleExp(double dval, oratext *outbuf, uword flags);
ub4 jznuFormatFloatExp(float fval, oratext *outbuf, uword flags);

/*
** NAME:
**  jznuFormatNumString
**
** DESCRIPTION:
**
**  Convert a JSON number to a canonical numeric string format. Attempts to
**  write the number without exponential notation, i.e. as a series of digits,
**  provided no digits of precision are lost, and no excess trailing zeros are
**  needed.  The radix is omitted if there are no fractional digits.  A minus
**  sign is prepended for negative values. 
**
**  The threshold is the maximum number of digits representable in
**  in the format, currently 40 digits of precision.
**
**  The input and output buffers can be the same.
**
** INPUTS:
**
**   oratext *data   : Input buffer
**   ub4      len    : Length of input buffer
**   oratext *outbuf : Output string, must be large enough for result
**
** OUTPUTS:
**   ub4           : length of output, 0 on error
*/
ub4 jznuFormatNumString(oratext *data, ub4 len, oratext *outbuf);

/*
** NAME:
**  jznuOraNumToString
**
** DESCRIPTION:
** Convert an Oracle Number to a decimal string.
** Returns the length of the result string, 0 on error.
**
** INPUTS:
**   ub1     *x      : Input number
**   ub4      len    : Length of input 
**   oratext *out    : Output string, must be large enough for result
**
** OUTPUTS:
**   ub4           : length of output, 0 on error
*/
ub4 jznuOraNumToString(ub1 *x, ub4 len, oratext *out);

/*
** NAME:
**  jznuOraDateToString
**
** DESCRIPTION:
** Convert an Oracle Date or Timestamp or Timestamp with Time Zone
** to an ISO-formatted string.
** Returns the length of the result string, 0 on error.
**
** INPUTS:
**   ub1     *x      : Input OraDate on-disk format image
**   ub4      len    : Length of input 
**   oratext *out    : Output string, must be large enough for result
**
** OUTPUTS:
**   ub4             : length of output, 0 on error
*/
ub4 jznuOraDateToString(ub1 *x, ub4 len, oratext *out);

/*
** NAME:
**  jznuOraDateToComponent
**
** DESCRIPTION:
** Convert an Oracle Date or Timestamp or Timestamp with Time Zone
** to a component timestamp.
**
** INPUTS:
**   ub1     *x      : Input OraDate on-disk format image
**   ub4      len    : Length of input 
**
** OUTPUTS:
**   sb8             : component (bit-field) timestamp
*/
sb8 jznuOraDateToComponent(ub1 *x, ub4 len);

/*
** NAME:
**  jznuOraDateToComponent
**
** DESCRIPTION:
** Convert an Oracle Date or Timestamp or Timestamp with Time Zone
** to a component timestamp.
**
** INPUTS:
**   sb8      tstamp : component (bit-field) timestamp
**   ub1     *x      : Output OraDate on-disk format image
**
** OUTPUTS:
**   ub4             : length of result (7 or 11 bytes)
*/
ub4 jznuComponentToOraDate(sb8 tstamp, ub1 *x);

/*
** NAME:
**  jznuDurationToString
**
** DESCRIPTION:
** Convert an Oracle Year/Month or Day/Second interval value
** to an ISO-formatted string.
** Returns the length of the result string, 0 on error.
**
** INPUTS:
**   ub1     *x      : Input interval on-disk format image
**   ub4      len    : Length of input 
**   oratext *out    : Output string, must be large enough for result
**
** OUTPUTS:
**   ub4             : length of output, 0 on error
*/
ub4 jznuDurationToString(ub1 *x, ub4 len, oratext *out);

typedef struct jznEventQueue jznEventQueue;

jznEventQueue  *jznEventQueueCreate(xmlctx *xctx);
void            jznEventQueueDestroy(jznEventQueue *sctx);
void            jznEventQueueReset(jznEventQueue *sctx);
sword           jznEventQueuePut(jznEventQueue *sctx, jznEventRecord *rec);
boolean         jznEventQueueGet(jznEventQueue *sctx, jznEventRecord *rec);
void            jznEventQueueBookmark(jznEventQueue *sctx);
void            jznEventQueueSetSource(jznEventQueue *sctx,
                                       jznEventSource *source);
sword           jznEventQueueStartRecording(jznEventQueue *sctx);
sword           jznEventQueueStopRecording(jznEventQueue *sctx);
sword           jznEventQueueRewind(jznEventQueue *sctx);
sb4             jznEventQueueCheckNext(jznEventQueue *sctx);
boolean         jznEventQueueIsRecording(jznEventQueue *sctx);
sword           jznEventQueueNext(jznEventQueue *sctx, jznEventRecord *rec);
jznEventSource *jznEventQueueToSource(jznEventQueue *sctx);

/* Return flags */
#define JZN_DETECT_REFUSED   0x0000
#define JZN_DETECT_ACCEPTED  0x0001
#define JZN_DETECT_COMPLETED 0x0002

/*
** Offer an event to the built-in complex type detector. This detects
** any/all of the following eJSON patterns:
**   BSON old and new formats
**   OSON extensions (which include some BSON forms)
**   FireBase syntax (not technically eJSON)
*/
#define JZN_DETECT_BSON_FORMS      0x0001
#define JZN_DETECT_OSON_FORMS      0x0002
#define JZN_DETECT_FIREBASE_FORMS  0x0004

ub4 jznEventQueueEjsonDetect(jznEventQueue  *sctx,
                             jznEventRecord *rec,
                             jznEventRecord *outrec,
                             uword           flags);

ub4 jznEventQueueBsonDetect(jznEventQueue  *sctx,
                            jznEventRecord *rec,
                            jznEventRecord *outrec);

ub4 jznEventQueueGeoDetect(jznEventQueue  *sctx,
                           jznEventRecord *rec,
                           jznEventRecord *outrec);

ub4 jznEventQueueOsonDetect(jznEventQueue  *sctx,
                            jznEventRecord *rec,
                            jznEventRecord *outrec);

/*
** SAX-like parsing interface
*/

typedef sword (*JsonSaxHandler)(void *ctx, jznEventRecord *rec);

sword JsonSaxRead(JsonEventSource *esrc, JsonSaxHandler cb, void *cbctx);

/*****************************************************************************
 * JSON Parser
 *
 * Parses textual JSON in UTF-8 or UTF-16 form, producing UTF-8
 * events. Optionally can convert non-Unicode character data to UTF-8.
 *****************************************************************************/

/*
** JSON parser context
*/
typedef struct jzn jzn;

/*
** The json.org standard doesn't allow scalars to be considered JSONs.
** The new RFC standard allows them.
**
** ### Legacy code should switch from ALLOW_ALL to ALLOW_STANDARD
** ### to stay compatible with old tests.
*/
#define JZN_ALLOW_STANDARD   (JZN_ALLOW_ALL & ~JZN_ALLOW_SCALAR_DOCUMENTS)
#define JZN_ALLOW_RFC        JZN_ALLOW_ALL

#define JZN_DISALLOW_DUPLICATES       0x00010000

#define JZN_USE_BSON_TYPES            0x00020000
#define JZN_USE_EXTENSION_TYPES       0x00040000
#define JZN_BSON_TYPE_PATTERNS        0x00080000

#define JZN_RUN_BULK_LOAD             0x00100000
#define JZN_RUN_VALIDATE_ONLY         0x00200000
#define JZN_RUN_SUPPRESS_ENDPAIR      0x00400000

#define JZN_DISALLOW_LOSSY            0x00800000
#define JZN_REPORT_ERRORS             0x01000000

#define JZN_USE_FIREBASE_TYPES        0x02000000

#define JZN_RUN_SHOW_COMMENTS         0x04000000

#define JZN_REPORT_LOCATION           0x8000000

/*
** NAME:
**   jznParserConfig
** PARAMETERS:
**   parser   (IN)  - JSON parser context
**   flags    (IN)  - flags specifying the allowed behaviors
** RETURNS:
**
** DESCRIPTION:
**   Set non-standard JSON parser behaviors.
** NOTES:
*/
void jznParserConfig(jzn *parser, ub4 flags);

/*
** NAME:
**   jznParserCreate
** PARAMETERS:
**   xctx    (IN)  - XML Context
** RETURNS:
**   (jzn *)  Pointer to new parser instance.
**   Returns a NULL on an out of memory condition.
** DESCRIPTION:
**   Create a JSON parser from an XML context. This allows the parser to create
**   a memory context hooking the XML context for exceptions and perhaps memory
**   callback functions. The parser will also take the current NLS global
**   pointer from the context, and will inherit the context's data encoding
**   language handle as a default encoding for inputs that aren't Unicode.
** NOTES:
**   The parser is a captive object of the XML context - it cannot exist
**   if the XML context is destroyed. However, the parser has its own memory
**   context that is not managed underneath that of the XML context, and so
**   the caller is obliged to destroy this object.
*/
jzn *jznParserCreate(xmlctx *xctx);
jzn *jznParserCreateEx(xmlctx *xctx, oramemctx *omctx, ub4 blksize);

/*
** NAME:
**   jznParserReset
** PARAMETERS:
**   parser   (IN)  - JSON parser context
** RETURNS:
**
** DESCRIPTION:
**   Reset a JSON, making it ready for another input. Closes any open
**   inputs. Clears the duplicate hash table (if any).
** NOTES:
**   Does not affect parser mode flags such as the allowed non-standard
**   behaviors.
*/
void jznParserReset(jzn *parser);

/*
** Return the size of the internal staging buffer used by the parser.
*/
ub4 jznParserGetSize(jzn *parser);

/*
** NAME:
**   jznParserDestroy
** PARAMETERS:
**   parser   (IN)  - JSON parser context
** RETURNS:
**
** DESCRIPTION:
**   Destroy a JSON parser, closing any inputs and freeing all memory
**   associated with it.
** NOTES:
*/
void jznParserDestroy(jzn *parser);

/*
** Return the line number and position within the line associated
** with the current event.
*/
void jznParserGetPosition(jzn *parser, ub4 *linenum, ub4 *linepos);

/*
** NAME:
**   jznParserSetBuffer
** PARAMETERS:
**   parser   (IN)  - JSON parser context
**   buffer   (IN)  - input buffer containing bytes to process
**   buflen   (IN)  - length of input buffer (in bytes)
** RETURNS:
**   status (JZNERR_OK if successful)
** DESCRIPTION:
**   Sets the input for a parser operation to the specified buffer.
**   The JSON parser needs to be in the reset state.
** NOTES:
**   Does not affect parser mode flags such as the allowed non-standard
**   behaviors.
*/
sword jznParserSetBuffer(jzn *parser, oratext *buffer, ub4 buflen);

/*
** NAME:
**   jznParserSetStream
** PARAMETERS:
**   parser   (IN)  - JSON parser context
**   stream   (IN)  - input stream
** RETURNS:
**   status (JZNERR_OK if successful)
** DESCRIPTION:
**   Sets the input for a parser operation to the specified stream.
**   The JSON parser needs to be in the reset state.
** NOTES:
**   Does not affect parser mode flags such as the allowed non-standard
**   behaviors.
** ### The code assumes the stream needs to be opened - is this a good idea?
*/
sword jznParserSetStream(jzn *parser, orastream *stream);

/*
** NAME:
**   jznParserSetFile
** PARAMETERS:
**   parser   (IN)  - JSON parser context
**   filename (IN)  - name/path to file
** RETURNS:
**   status (JZNERR_OK if successful)
** DESCRIPTION:
**   Sets the input for a parser operation to the specified file.
**   The JSON parser needs to be in the reset state.
** NOTES:
**   Does not affect parser mode flags such as the allowed non-standard
**   behaviors.
** ### The code assumes it's OK to open the file immediately. This is
** ### opposite of streams, where the open is deferred until later.
** ### This allows a file open error to be thrown immediately.
*/
sword jznParserSetFile(jzn *parser, oratext *filename);

/*
** NAME:
**   jznParserSetEncodingMode
** PARAMETERS:
**   parser   (IN)  - JSON parser context
**   csmode   (IN)  - Input encoding mode
** RETURNS:
**
** DESCRIPTION:
**   Sets the NLS character set to use for the input. This is necessary
**   if the parser needs to convert from a non-Unicode database character
**   set.
** NOTES:
**   The JSON parser needs to be in the reset state. This interface can be
**   called either before or after setting an input buffer, stream, or file.
**   Once parsing begins, calling this function has no effect.
*/
void jznParserSetEncodingMode(jzn *parser, ub2 csmode);

/*
** NAME:
**   jznParserTerminate
** PARAMETERS:
**   parser   (IN)  - JSON parser context
** RETURNS:
**
** DESCRIPTION:
**   Terminate an in-progress parsing operation, closing any input
**   source. Does not reset the parser for another parse operation.
** NOTES:
**   This interface may be useful if a caller needs to clean up resouces
**   such as open file or stream handles, but needs to keep the parser in
**   some sort of final state (e.g. because it has an error, event, or
**   other state that needs to be preserved).
*/
void jznParserTerminate(jzn *parser);

/*
** NAME:
**   jznParserGetEvent
** PARAMETERS:
**   parser   (IN)  - JSON parser context
**   rec      (OUT) - Event record
**   csid     (IN)  - desired output character set
** RETURNS:
**   status
** DESCRIPTION:
**   Get the next event from the JSON parser. If csid is not for
**   AL32UTF8, does a character set conversion to the desired csid.
** NOTES:
*/
sword jznParserGetEvent(jzn *parser, jznEventRecord *rec, sb2 csid);

/*
** NAME:
**   jznParserConvertEvent
** PARAMETERS:
**   parser   (IN)  - JSON parser context
**   rec      (OUT) - Event record
**   csid     (IN)  - desired output character set
** RETURNS:
**   status
** DESCRIPTION:
**   Converts the event to the specified character set. If 0, converts
**   to the "native" character set of the parser's XmlCtx.
** NOTES:
*/
sword jznParserConvertEvent(jzn *parser, jznEventRecord *rec, sb2 csid);

/*
** NAME:
**   jznGetErrMessage
** PARAMETERS:
**   parser    (IN)  - Parser context
** RETURNS:
**   (oratext *)  - Formatted error message
** DESCRIPTION:
**   Returns a string pointer to a formatted message representing the
**   current error state. The formatted message contains a file name
**   and line number, plus the error text and any error arguments.
** NOTES:
**   The buffer returned is internal to the parser - the buffer is
**   reused if the parser is reset, so the caller is obliged to copy
**   the error message if needed.
*/
oratext   *jznGetErrMessage(jzn *parser);

/*
** NAME:
**   jznGetErrorCode
** PARAMETERS:
**   parser    (IN)  - Parser context
** RETURNS:
**   (jsonerr)  - Returns an error code
** DESCRIPTION:
**   Returns a code representing the most recently encountered error
**   condition. Returns JZNERR_OK if no errors have occurred since the
**   most recent reset operation.
** NOTES:
** ### Returns the jsonerr type - why?
*/
jsonerr    jznGetErrCode   (jzn *parser);

/*
** Create (or reuse) a user sub-context of a specified size.
*/
void *jznParserMakeUserContext(jzn *parser, size_t sz);

/*
** Return the user sub-context for this parser (or NULL if none).
*/
void *jznParserGetUserContext(jzn *parser);

/*
** Return the XML context used by this parser
*/
xmlctx *jznParserGetXmlContext(jzn *parser);

/*
** ### Eventually move to a customer-facing header? Or obsolete now?
*/
jzn     *jznCreate         (xmlctx *xctx, ...);
jzn     *jznCreateVA       (xmlctx *xctx, va_list va);

/*
** Return the parser as an event source
*/
jznEventSource *jznParserToSource(jzn *parser);

/*
** Return the parser backing an event source.
** Returns NULL if the source is not backed by a parser.
*/
jzn *jznParserFromSource(jznEventSource *jsrc);

/*
** Force the parser to use an external fetch buffer. The buffer can be
** a stack variable but only if the parser will be run to exhaustion
** while the buffer remains in scope. Bufsize should be at least 4000
** bytes plus 16 bytes of padding. The caller can use this to set
** the buffer back to a NULL if desired.
*/
void jznParserSetFetchBuffer(jzn *parser, void *buffer, ub4 bufsize);

/*
** Memory stream writer
*/
typedef struct jznuStreamWriter jznuStreamWriter;

/*
** Create a stream writer that manages a memory buffer to receive
** writes from a stream. Creates the orastream around itself.
*/
jznuStreamWriter *jznuStreamWriterCreate(xmlctx *xctx, oramemctx *omctx);

/*
** Free a stream writer and close the stream.
*/
void jznuStreamWriterDestroy(jznuStreamWriter *swriter);

/*
** Get the stream supported by the stream writer.
*/
orastream *jznuStreamWriterGet(jznuStreamWriter *swriter);

/*
** Get the buffer and buffer length written to a stream writer.
*/
void *jznuStreamWriterGetBuf(jznuStreamWriter *swriter, size_t *len);

/*
** Create a stream writer attached to a print serializer.
** The stream writer can be used to stream bytes to an extensible buffer.
** This is a convenience API that uses the printer as a source of memory.
*/
jznuStreamWriter *jznuPrintGetStreamWriter(jznuPrintCtx *pctx);

#endif                                              /* JZNEV_ORACLE */
