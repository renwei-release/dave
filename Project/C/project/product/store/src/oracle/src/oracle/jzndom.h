/* Copyright (c) 2015, 2025, Oracle and/or its affiliates.*/
/* All rights reserved.*/

/*
   NAME
     jzndom.h - JSON DOM interfaces

   DESCRIPTION
     Allow manipulation of a JSON object tree as a DOM.

   RELATED DOCUMENTS

   EXPORT FUNCTION(S)
     Contains the following definitions:
     - DOM structures and function-pointer interface.
     - DOM writer: a facility for creating a DOM by pushing events to a builder
     - DOM reader: a facility for treating a DOM as an event source

   EXAMPLES

   NOTES
     This is a public (to customers) header file and world-viewable.
     The file can include only public headers.

   MODIFIED   (MM/DD/YY)
   apfwkr      10/30/24 - CI# 37230449 of dmcmahon_bug-36788895 from main
   apfwkr      06/06/24 - CI# 36706798 of dmcmahon_bug36357136 from main
   zliu        01/17/25 - Bug37486754: JsonDomAryMinMaxJt,JsonDomAryAggRetSca
   dmcmahon    01/07/25 - Bug 37454448: JZNANY_FLAG_PRESERVE_[TYPES|ORDER]
   zliu        12/20/24 - Bug 37429418  JZNDOM_FIELD_POS_SET
   dmcmahon    11/05/24 - Bug 37249695: sb2val/ub2val for jznScalarVal
   dmcmahon    10/31/24 - Bug 37236015: lint and tab cleanup
   zliu        10/08/24 - Bug 37204122: dom api to push down range cmp/in-clause
   zliu        09/10/24 - Bug 37054087: add JZNDOM_OCT_NOT_4_UPD
   zliu        08/30/24 - Bug 37020310: JZNDOMCHK_ALLOW_SAME_FAM
   dmcmahon    08/22/24 - Bug 36979516: add jznDomWriterGetCount()
   dmcmahon    08/16/24 - Bug 36958159: add jznDomWriterGetDoc()
   huiz        07/31/24 - Bug 36889754: add JZNDOM_UNKNOWN
   zliu        07/08/24 - Bug 36817752: jznDomChkJModifier()
   dmcmahon    06/17/24 - Bug 36761296: JZNANY_FLAG_NULL_ON_EMPTY
   dmcmahon    06/06/24 - Bug 36732069: jznDomScalarArray()
   dmcmahon    12/30/23 - Bug 36148907: jznScalarVal fields for opaque
   zliu        12/18/23 - bug 36132471: vector type into json xdk JZNVAL_VECTOR
   dmcmahon    10/04/23 - Bug 35875900: jznDomSetRoot return status code
   zliu        08/15/23 - Bug 35746966: JZNDOM_OCT_TREE_SZ_UB4
   zliu        07/04/23 - Bug 35570236: JZNDOM_PRINT_HDL_RAW_ASOSON
   dmcmahon    06/28/23 - Bug 34841184: JsonDomNumberFrom(Long|Double)
   dmcmahon    06/27/23 - Bug 35533687: Reader/Writer oramemctx constructors
   zliu        05/20/23 - bug 35431180: share oson decoder ctx for json eval
   zliu        05/20/23 - bug 35431180: dom api of reset with flag parameter
   dmcmahon    05/05/23 - Bug 35284528: JZNDOM_COMPARE_FLAG_TEMP
   dmcmahon    03/15/23 - Bug 35185835: comments on JsonDomGetFieldValue
   zliu        03/08/23 - Bug 35219425: support ub2_field_name_length for
   dmcmahon    03/07/23 - Bug 35113192: xmlctx parameter for TSTZ formatting
   dmcmahon    02/15/23 - Bug 35088293: JZNDOM_COMPARE_FLAG_TYPECHECK
   dmcmahon    01/20/23 - Bug 35002198: JZNDOM_COMPARE_FLAG_BRACKET
   zliu        09/19/22 - Bug 34615330: jznDomCompareWithTypeBracket()
   dmcmahon    08/22/22 - Bug 34528802: JsonDomGetScalarType()
   dmcmahon    08/09/22 - Bug 34477101: JsonDomReplaceField()
   dmcmahon    08/05/22 - Bug 34470402: JsonDomSetScalar()
   dmcmahon    08/03/22 - Bug 34457920: JsonDomCopyScalar()
   dmcmahon    07/12/22 - Bug 34373161: JZNANY node-type match flags
   dmcmahon    06/06/22 - Bug 34248833: JZNDOM_NODE_HASH_LENGTH == 16
   dmcmahon    04/30/22 - Bug 34125921: JsonDomAnyCompare()
   zliu        04/07/22 - Bug 34050578: add JZNDOM_OCT_COMPACT_CLUSTER
   dmcmahon    03/11/22 - Bug 33948659: jznDomUnwrapRootArray
   sriksure    02/14/22 - Bug 33845235: Cast mode of schema validation
   zliu        01/18/22 - Bug 33766164:JZNDOM_OCT_SHR_SIMP_NODES
   dmcmahon    09/09/21 - Bug 33334358: lpxmemctx should not be exposed here
   dmcmahon    06/23/21 - Bug 33061052: comments about hash
   dmcmahon    06/25/21 - Bug 33048916: new DOM array bulk methods
   dmcmahon    06/19/21 - Bug 33022120: Add JsonDomReadAnyType()
   dmcmahon    06/12/21 - Bug 32995128: Array sort as a DOM method
   dmcmahon    06/10/21 - Bug 32988631: JsonDomSortArray
   dmcmahon    06/08/21 - Bug 32973954: max depth/size for JsonDomAnyType
   dmcmahon    06/04/21 - Bug 32967328: JsonDomAnyType()
   sriksure    05/27/21 - Bug 32934766: Add JsonDomArrayIsUnique()
   jvalera     05/21/21 - Bug 32903599: Add JZN_UNKNOWN_DOM
   dmcmahon    05/19/21 - Bug 32904319: jznDomHash
   dmcmahon    05/19/21 - Bug 32904319: JsonDomHash()
   sriksure    03/08/21 - Bug 32593809: Bulk loading from OSON stream
   sriksure    02/24/21 - Bug 32409241: Support JZN_FORMAT in JsonDomCreate()
   dmcmahon    01/14/21 - Bug 32383277: add JZNDOM_TRACK_POSITIONS
   sriksure    10/09/20 - Bug 31996145: Add JsonLoadDom(), JsonSerializeDom()
   dmcmahon    08/31/20 - Bug 31820651: add JsonDomLoadItem
   dmcmahon    08/12/20 - Bug 31748678: new DOM constructors
   dmcmahon    07/17/20 - Bug 31637902: New DOM methods
   zliu        07/24/20 - Bug 31694034:
                          JZNDOM_NO_SHARE_OBJ_DEFN,JZNDOM_USE_UB4_OFFSET
   dmcmahon    07/15/20 - Fix jznDomM_SetError again
   dmcmahon    07/08/20 - Add dummy_jznoctFieldName
   dmcmahon    07/06/20 - Bug 31584233: prop up jznDomM_SetError for clsjson.c
   dmcmahon    06/27/20 - Store fldnm offset in jznDomObjFieldName dead space
   zliu        06/10/20 - document error return case
   dmcmahon    06/05/20 - DOM reader/writer captive constructors move to jzn.h
   dmcmahon    05/04/20 - Bug 31292369: Constructors may return NULLs
   sriksure    03/24/20 - Bug 31075323: Add JZNVAL type, setter for LdiInterval
   sriksure    02/20/20 - Bug 30918101: Add new DOM method, remove varargs
                          from GetScalarInfo
   sriksure    02/20/20 - Bug 30918101: Unpickle native binary values
   zliu        01/26/20 - Bug 30840791,30857500: jznDomWriterGetWrtCnt()
   zliu        09/11/19 - Bug 30300159: JZNDOM_DOM_FOR_OSON_PUDP_REP_ONLY
   dmcmahon    08/12/19 - bug 30180010: add jznDomReaderSetFlags()
   bhammers    05/17/19 - bug 29794416: JZNDOM_DISALLOW_SCALARS
   dmcmahon    04/24/19 - bug 29687763: add JZNDOM_VALIDATE_STRINGS
   zliu        04/08/19 - Bug 29619119: JZNDOM_DOM_FOR_VALIDATION
   dmcmahon    04/07/19 - bug 29611018: JZNVAL_ID
   dmcmahon    04/04/19 - Bug 29599702: JsonDomFreeNode()
   dmcmahon    03/28/19 - Bug 29554518: fix more broken macros
   zliu        03/13/19 - Bug 29514638:enable JZNVAL_CANONICAL_FLOAT &
                          JZNVAL_CANONICAL_DOUBLE
   bhammers    03/19/19 - correct macros, indentation, 80 chars per line
   dmcmahon    02/27/19 - Fix macro JsonDomDeleteFieldByName()
   dmcmahon    02/22/19 - Fix macro JsonDomCreateStream()
   zliu        12/04/18 - Bug 28968363: add JZNOCT_FIELD_NM_PARTIAL
   sriksure    09/27/18 - Bug 28717127: Rationalize includes in public jzn*.h
   zliu        09/26/18 - Bug 28708424: add jznoctLoadInMemDomFromInputOSON_WF()
   dmcmahon    09/13/18 - Bug 28645808: fix lint problems
   dmcmahon    09/05/18 - bug 28603908: jznDomScalarToString
   sriksure    08/23/18 - Bug 28546488: XDK C JSON APIs enhancements
   zliu        06/28/18 - Parital OSON update add JZNDOM_DOM_FOR_OSON_PUPD
   zliu        04/10/18 - Bug 27896443 add JsonDomGetDescendantNodes()
   zliu        01/14/18 - Bug 27390960: add JZNDOM_FIELD_USED
   zliu        11/28/17 - number of distinct fields increasing from ub2 to ub4
   dmcmahon    10/21/17 - Bug 26739606: comment formatting
   zliu        08/12/17 - add JZNDOM_DOM_FOROSON_ENC
   zliu        06/22/17 - fix bug 26331370
   dmcmahon    04/25/17 - Bug 25954542: oradslen_jznScalarVal typo
   dmcmahon    03/07/17 - add JZNDOM_PRESERVE_NODES
   zliu        11/29/16 - set oson support
   dmcmahon    11/09/16 - Bug 25072880: add jznCreateDomVA()
   zliu        08/02/16 - no noinlineleaf mode
   zliu        06/15/16 - add JsonDomNodeVisitor() api
   zliu        06/15/16 - add JsonDomPrintNode() api
   zliu        06/10/16 - add jznoct_dom_visit()
   zliu        06/05/16 - add JZNDOM_BATCH_API
   zliu        06/04/16 - supporting jzn0dom construction from oson binary
   zliu        06/03/16 - JsonDomCreateArray(),JsonDomCreateObject() size param
   zliu        06/01/16 - add JZNDOM_REUSE_NAMES
   dmcmahon    02/17/16 - Add jznDomWriterReadSource()
   zliu        01/25/16 - add inline leaf val encoding for oson
   zliu        01/19/16 - streaming encoding for oson
   dmcmahon    10/21/15 - Bug 22082373 - Add JsonDomSetXmlContext()
   zliu        10/08/15 - add JZNDOM_NO_FRC_UB1_HID
   dmcmahon    07/01/15 - Bug 21356957 - add JZNDOM_CHECKLINKS
   dmcmahon    04/30/15 - Bug 20992659 - jznDomSortFields()
   bhammers    03/31/15 - added JsonDomRenameField
   dmcmahon    03/12/15 - Creation

*/

#ifndef JZNDOM_ORACLE
# define JZNDOM_ORACLE

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

#ifndef JZNEV_ORACLE
# include <jznev.h>
#endif

/*---------------------------------------------------------------------------
                            PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/*
** Public Types
*/

#define JsonDomDoc       jznDomDoc          /* JSON DOM Document Container */
#define JsonDomNode      jznDomNode         /* Abstract JSON Node Type     */
#define JsonDomObject    jznDomObject       /* JSON Object Type            */
#define JsonDomArray     jznDomArray        /* JSON Array  Type            */
#define JsonDomScalar    jznDomScalar       /* JSON Scalar Type            */

/*
** Forward Declarations
*/

/*
** JSON DOM Conatiner
*/
typedef struct jznDomDoc jznDomDoc;

/*
** JSON DOM nodes, these are abstract classes
** JSON Scalar DOM node, a derivation type of jznDomNode
** JSON Object DOM node, a derivation type of jznDomNode
** JSON Array DOM node, a derivation type of jznDomNode
*/

typedef struct jznDomNode   jznDomNode;

typedef struct jznDomScalar jznDomScalar; /* Castable to jznDomNode */

typedef struct jznDomObject jznDomObject; /* Castable to jznDomNode */

typedef struct jznDomArray  jznDomArray;  /* Castable to jznDomNode */


#define JZN_SCALAR_VAL_SZ  256 /* ### Not sure why we define this? ### */
#define JZN_ORA_NUM_MAXSZ  22  /* ### Not sure why this is public? ### */

/*****************************************************
 * JSON DOM API
 *****************************************************/

/*
** DOM Scalar Value Output Structure
*/
typedef struct jznScalarVal
{
  jznvaltype     type_jznScalarVal;
  jzn_sval_union uval_jznScalarVal;

/* JZNVAL_STRING */
#define len_jznScalarVal               uval_jznScalarVal.val.valuelen
#define val_jznScalarVal               uval_jznScalarVal.val.value

/* JZNVAL_BINARY */
#define binval_jznScalarVal            uval_jznScalarVal.bval.value
#define binlen_jznScalarVal            uval_jznScalarVal.bval.valuelen

/* JZNVAL_VECTOR */
#define vecval_jznScalarVal            uval_jznScalarVal.bval.value
#define veclen_jznScalarVal            uval_jznScalarVal.bval.valuelen

/* JZNVAL_ANY_TYPE */
#define subtype_jznScalarVal           uval_jznScalarVal.opaque.subtype
#define flags_jznScalarVal             uval_jznScalarVal.opaque.flags
#define dtype_jznScalarVal             uval_jznScalarVal.opaque.dtype

/* JZNVAL_SHORT, JZNVAL_USHORT - machine native sb2, ub2 */
#define sb2val_jznScalarVal            uval_jznScalarVal.ival.shortval
#define ub2val_jznScalarVal            uval_jznScalarVal.uval.shortval
/*
** JZNVAL_FLOAT16, JZNVAL_BFLOAT16 - canonical ub2, platform-endian, in-line
** This is done so that comparisons can be done on the ub2. C has no native
** 16-bit floating-point types, but SQL doesn't have a representation either.
*/

/* JZNVAL_SINT, JZNVAL_UINT - machine native sb4, ub4 */
#define sb4val_jznScalarVal            uval_jznScalarVal.ival.intval
#define ub4val_jznScalarVal            uval_jznScalarVal.uval.intval

/* JZNVAL_SLONG, JZNVAL_ULONG - machine native sb8, ub8 */
#define sb8val_jznScalarVal            uval_jznScalarVal.ival.longval
#define ub8val_jznScalarVal            uval_jznScalarVal.uval.longval

/* JZNVAL_FLOAT - machine native float */
#define flt_jznScalarVal               uval_jznScalarVal.fval.ftval

/* JZNVAL_DOUBLE - machine native double */
#define db_jznScalarVal                uval_jznScalarVal.dval.fpval

/* JZNVAL_TSTAMP - structured timestamp */
#define tstamp_jznScalarVal            uval_jznScalarVal.tval.dtval

/* JZNVAL_UUID/JZNVAL_OID - in-line ID */
#define id_jznScalarVal                uval_jznScalarVal.pval.oidval

/* JZNVAL_DECIMAL - in-line Decimal128 */
#define mem_jznScalarVal               uval_jznScalarVal.xval.mem128

/* JZNVAL_CANONICAL_FLOAT, JZNVAL_CANONICAL_DOUBLE
** Oracle native binary/canonical float, double
*/
#define flt_orabin_jznScalarVal        uval_jznScalarVal.obinval.obinptr_obinval
#define db_orabin_jznScalarVal         uval_jznScalarVal.obinval.obinptr_obinval

/* JZNVAL_ORA_NUMBER - Oracle native number */
#define oranumval_jznScalarVal         uval_jznScalarVal.obinval.obinptr_obinval
#define oranumlen_jznScalarVal         uval_jznScalarVal.obinval.oblen_obinval

/* JZNVAL_ORA_TIME - Oracle native time */
#define oratimeval_jznScalarVal        uval_jznScalarVal.obinval.obinptr_obinval
#define oratimelen_jznScalarVal        uval_jznScalarVal.obinval.oblen_obinval

/* JZNVAL_ORA_DATE - Oracle native date */
#define oradateval_jznScalarVal        uval_jznScalarVal.obinval.obinptr_obinval
#define oradatelen_jznScalarVal        uval_jznScalarVal.obinval.oblen_obinval

/* JZNVAL_ORA_TIMESTAMP - Oracle native timestamp */
#define oratimestampval_jznScalarVal   uval_jznScalarVal.obinval.obinptr_obinval
#define oratimestamplen_jznScalarVal   uval_jznScalarVal.obinval.oblen_obinval

/* JZNVAL_ORA_TIMESTAMPTZ - Oracle native timestamp with timezone */
#define oratimestamptzval_jznScalarVal uval_jznScalarVal.obinval.obinptr_obinval
#define oratimestamptzlen_jznScalarVal uval_jznScalarVal.obinval.oblen_obinval

/* JZNVAL_ORA_YEARMONTH_DUR - Oracle native year month interval */
#define orayrmnval_jznScalarVal        uval_jznScalarVal.obinval.obinptr_obinval
#define orayrmnlen_jznScalarVal        uval_jznScalarVal.obinval.oblen_obinval

/* JZNVAL_ORA_DAYSECOND_DUR - Oracle native day second interval */
#define oradsval_jznScalarVal          uval_jznScalarVal.obinval.obinptr_obinval
#define oradslen_jznScalarVal          uval_jznScalarVal.obinval.oblen_obinval

} jznScalarVal;

/*
** JSON DOM nodes come in three types:
**   Scalars: number, string, true/false/null, + extensions
**   Objects: containers with unordered field/value pairs
**   Arrays:  containers with ordered values
*/

typedef enum
{
  JZNDOM_UNKNOWN = 0,
  JZNDOM_SCALAR  = 1,
  JZNDOM_OBJECT  = 2,
  JZNDOM_ARRAY   = 3
} jznnodetype;

/*
** Hash id of a JSON object field name (big-endian ub4 encoded)
*/
typedef ub4 jznoct_hashidt;

/*
** Field name offset (for OSON encoding)
*/
typedef ub4 jzndom_fnmOff;

typedef struct jznDomObjFieldName
{
  oratext       *fnm_jznDomObjFieldName;           /* JSON object field name */
  ub4            len_jznDomObjFieldName;        /* length of JSON field name */
  jznoct_hashidt hashid_jznDomObjFieldName;   /* hashId of object field name */
   /* hashid_jznDomObjFieldName shall be set
    * so that jznOctLocateFieldId() for OSON DOM via  JsonDomGetFieldValue()
    *  can use the hashid look up directly
    * instead of computing from scratch to improve performance.
    * Optimization using hashid is also true for
    * in-memory JSON DOM via jzn0DomFindField() using hashid match first.
    */
  /*
  ** ### Ideally we would not expose these internal members to customers.
  ** ### The size of this structure should then be 16 bytes.
  */

  /* Bug 37429418 fix:
  ** id of the field name within an instance OSON
  ** It is 1 based instead of 0 based. 0 means the id info is not available.
  ** Note ub4 shall be the same as jznoct_fnmOff type, which is ub4.
  ** This is returned by DOM call:
  ** JsonDomGetFieldValue(doc, (jznDomObject *)curnode, fnm);
  ** for OSON DOM: JsonDomGetFieldValue()
  **  calls jznOctLocateFieldId() which uses
  ** cahced fnm->id_jznDomObjFieldName if available 
  ** or 
  ** pre-computed fnm->hashid_jznDomObjFieldName in shared cursor query plan
  ** to do binary search of fields with that paritcular field name <fnm>
  ** For setOSON case, it also uses fnm->idInSetOSON_jznDomObjFieldName.
  ** jznEngSetUpModifiableFnmFromStep() calls JsonDomGetFieldValue().
  ** Each jznEng has jznEngFsm  struct that has an array of jznDomObjFieldName
  ** that is used to cache field name ids when traversing oson dom at per
  ** instance level.
  */
  ub4            id_jznDomObjFieldName;
  /*
  ** As total number of distinct field inceases from ub2 to ub4, so
  ** we can not use ub2 anymore, so we declare id_jznDomObjFieldName as ub4
  ** above and use ub2hash_jznDomObjFieldName to store ub2 hash.
  */
  ub2           ub2hash_jznDomObjFieldName;
#define id_ub2_jznDomObjFieldName ub2hash_jznDomObjFieldName

  ub1           flag_jznDomObjFieldName;                             /* flag */
#define JZNDOM_FIELD_NOT_EXISTS_IN_THIS_SET 0x1
#define JZNDOM_FIELD_USED                   0x2
#define JZNDOM_FIELD_POS_SET                0x4
  /* JZNDOM_FIELD_POS_SET indicating pos_jznoctFieldName is valid */
  ub1           ub1hash_jznDomObjFieldName;                 /* 1 byte hashid */

  /*
  ** In OSON set encoding for IMCJ usecase, a set of JSON instances
  ** is encoded into OSON instances with shared field name buffers, so
  ** we need to use setId_jznDomObjFieldName to get its field id.
  ** It is 1 based instead of 0 based. 0 means the id info is not available.
  */
  ub2               idInSetOSON_jznoctFieldName;

  /*
  ** Array Position within the array of fieldname structures 
  ** (sideband array of unique fields) prepared by jznp_qjsnBldFidsForPathExpr()
  ** In step structures for path expressions, this is used as the slot number
  ** for a sideband array of unique fields.
  */
  ub2               pos_jznoctFieldName;

  jzndom_fnmOff     nmoff_jznoctFieldName; /* offset into the field name seg */

} jznDomObjFieldName;

// ### Temporarily prop up existing readers
#define idInSetOSON_jznDomObjFieldName idInSetOSON_jznoctFieldName
#define jznoctFieldName jznDomObjFieldName

/*
** This structure is used to read field name/value pairs from the DOM.
** The values are always nodes. The field name structure contains the
** name string and length, plus some fields to treat the field name
** via hash/id codes and to deal with set-based DOMs (which share
** the field name information across DOM instances).
*/
typedef struct jznDomNameValuePair
{
  jznDomObjFieldName   name_jznDomNameValuePair;
  jznDomNode          *node_jznDomNameValuePair;
} jznDomNameValuePair;

// ### Why didn't we return a pointer to jznDomObjFieldName in the above
// ### structure? It makes the structure 40 bytes in size, whereas in the
// ### DOM itself it's stored as a pair of pointers (16 bytes). This also
// ### forces exposure of the large structure jznDomObjFieldName which is
// ### heavy with internal fields.

#define JZNDOMNODE_NULL ((jznDomNode *)NULL)

/* ### Unfortunately exposed through ordinary DOM ### */
typedef struct jznosonFile jznosonFile;

/*
** jznDomDoc is the container for the JSON DOM tree. It is the
** first argument to all jznDomAPI methods.
** The constructor for jznDomDoc is jznCreateDom() which can creates
** different implementations of the same jznDomAPI interface
*/
typedef struct jznDomAPI jznDomAPI;

struct jznDomDoc
{
  /*
  ** DOM method callback functions (implementation-specific)
  */
  jznDomAPI *mthds_jznDomDoc;
  /*
  ** Required members common to all DOMs
  */
  xmlctx    *xctx_jznDomDoc;     /* XML context           */
  jsonerr    errcode_jznDomDoc;  /* Error code            */
  ub4        modcount_jznDomDoc; /* Modification count    */
};

typedef enum jznDomTyp
{
  JZN_UNKNOWN_DOM = 0, /* Unknown DOM type                               */
  JZN_INMEM_DOM   = 1, /* mutable DOM backed by in-memory structures     */
  JZN_OSON_DOM    = 2  /* immutable DOM backed by OSON serialized buffer */
} jznDomTyp;

/*
** Used for DOM visitor callback func JsonDomNodeVisitor
*/
typedef struct JsonDomVisitorInfo
{
  jznDomDoc       *jdoc_JsonDomVisitorInfo;
  jznDomNode      *node_JsonDomVisitorInfo;
  jznnodetype      ntyp_JsonDomVisitorInfo;
                                 /* node type of the node_JsonDomVisitorInfo */
  ub4              tot_JsonDomVisitorInfo;
       /* total children if the node_JsonDomVisitorInfo is object/array node */
  jznScalarVal    *scal_JsonDomVisitorInfo;
                /* scalar info if the node_JsonDomVisitorInfo is scalar node */

} JsonDomVisitorInfo;

typedef void (*JsonDomVisitorFunc)(JsonDomVisitorInfo *vinfo, void *appctx) ;

/*
** DOM API can fetch an array of obj/array children with size
** JZNDOM_BATCH_API to improve performance
*/
#define JZNDOM_BATCH_API 128

/*
** JSON DOM Constructor flags
**
** Can be used as flags argument in call to jznCreateDom() or JZN_MODE
** varargs argument in call to JsonDomCreate().
*/

#define JZNDOM_FLAGS_NONE                  0x00000000

/*
** By default, jznDomDoc can be created once and be re-used for many JSON
** doc instances so that memory resources can be allocated once re-used to
** cycle through many object instances. Setting flag JZNDOM_RECYCLE_ALL
** forces cleaning up resources on each JSON object instance.
*/
#define JZNDOM_RECYCLE_ALL                 0x00000001

/*
** By default, oson binary sorts field ids of an object so that
** locating field within an object caqn be very efficient via binary
** search. However, this may be at the expense of encoding the oson
** and looses the original order of fields within the object (although
** order of fields within json object is undefined)
** Setting JZNDOM_OCT_NOSORTFID forces no sorting of field ids for encoding.
*/
#define JZNDOM_OCT_NOSORTFID               0x00000002

/*
** By default, all JSON number are stored as ORA number format.
** However, for internal testing purpose, this JZNDOM_NUM_AS_STR stores
** JSON number as string.
*/
#define JZNDOM_NUM_AS_STR                  0x00000004

/*
** By default, float and double are stored as native binary format aka
** canonical format. This flag requests that for reading via
** JsonDomGetScalarInfo(), the types JZNVAL_CANONICAL_DOUBLE and
** JZNVAL_CANONICAL_FLOAT are converted to JZNVAL_DOUBLE and JZNVAL_FLOAT
** when delivered to a DOM read operation.
*/
#define JZNDOM_NUM_AS_IEEE                 0x00000008

/*
** Build set oriented DOM for in-memory Columnar JSON processing. In this mode,
** a SET of JSON document instances are encoded together by sharing
** field-name dictionary and path-value dictionary. However, each JSON doc
** still has its navigational tree component seperated.
*/
#define JZNDOM_SET4IMJ                     0x00000010

/*
** Bypass internal free lists of nodes. This may give a minor boost to
** performance while sacrificing memory efficiency.
*/
#define JZNDOM_NOFREELIST                  0x00000020

/*
** ### This will replace JZNDOM_NUM_AS_STR, which will become the default.
** ### This flag will eventually disable string numbers.
*/
#define JZNDOM_NUM_AS_ORA                  0x00000040

/*
** Ensure that non-orphan nodes are not linked multiple times.
** Prevent cyclic references in the DOM tree.
*/
#define JZNDOM_CHECKLINKS                  0x00000080

/*
** NOT to Force to Use UB1 hash id for oson hash id
** This flag is DEPRECATED, no usage
*/
#define JZNDOM_NO_FRC_UB1_HID              0x00000100

/*
** OSON streaming encoding, that is each child of an obj/array encoded
** in streaming fashion without tree node table
** This flag is DEPRECATED, no usage
*/
#define JZNDOM_STREAM_ENCODED              0x00000200

/*
** For partial OSON update, if there is only replacement needed without
** extending segments, then we do partial OSON doc update. Otherwise, we do
** full OSON doc update.
*/
#define JZNDOM_DOM_FOR_OSON_PUDP_REP_ONLY  0x00000400

/*
** For JSON DOM construction over a set of JSON docs, re-use hash table
** for field names
*/
#define JZNDOM_REUSE_NAMES                 0x00000800

/*
** For JSON DOM mutations preserve freed nodes (e.g. for PL/SQL).
** This is memory-heavy but supports PL/SQL's reference semantics.
*/
#define JZNDOM_PRESERVE_NODES              0x00001000

/*
** This in-memory DOM is used for OSON encoding purpose
*/
#define JZNDOM_DOM_FOROSON_ENC             0x00002000

/*
** Support partial OSON updates on persistent OSON.
*/
#define JZNDOM_DOM_FOR_OSON_PUPD           0x00004000

/*
** Validate incoming OSON bytes and reject possible malicious bytes.
*/
#define JZNDOM_DOM_FOR_VALIDATION          0x00008000

/*
** Ensure strings and field names are correct UTF-8 sequences.
*/
#define JZNDOM_VALIDATE_STRINGS            0x00010000

/*
** Reject if JSON is a scalar value
*/
#define JZNDOM_DISALLOW_SCALARS            0x00020000

/*
** As an event source, preserve extended types
*/
#define JZNDOM_EMIT_EXTENDED_TYPES         0x00040000

/*
** No array of obj sharing defintion optimization during oson encoding
*/
#define JZNDOM_NO_SHARE_OBJ_DEFN           0x00080000

/*
** Use ub4 offset for tree offset always
*/
#define JZNDOM_USE_UB4_OFFSET              0x00100000

/*
** Allow nodes to be mounted on a tree multiple times.
** Effectively allows the DOM to be a directed acyclic graph.
*/
#define JZNDOM_MULTI_PARENT                0x00200000

/*
** Limits the DOM operations to those needed for initial construction.
** Disables remove/unlink operations, rename/move, and replacing values.
*/
#define JZNDOM_CONSTRUCTOR_ONLY            0x00400000

/*
** Adds a node hash table to track the parse position of each DOM node.
** Costly, use only when needed. Only works when constructing a DOM
** from a textual parse. Cannot track later mutations.
*/
#define JZNDOM_TRACK_POSITIONS             0x00800000

/* during oson encoding, all simple 
 * JSON scalar node representing json null, true,
 * false, 0, "" are shared in oson image, 
 * All empty object {} and all empty array [] are shared in oson image.
 * This makes resulting OSON size as compact/small as possible.
 *
 * {"a":null, "b":null}   json scalar null is shared.
 * {"a":[], "b":[]}       json empty array is shared.
 */
#define JZNDOM_OCT_SHR_SIMP_NODES         0x01000000

/* during oson encoding, sharing JSON scalar node of the same leaf value
 * if their parents have the same key name.
 * This makes resulting OSON size as compact/small as possible.
 *
 * [ {"a": "done", "b": "finished" }, {"b": "finished"}]
 * leaf node "finished" is shared in oson image.
 */
#define JZNDOM_OCT_SHR_PA_SCALAR           0x02000000

/* during oson encoding, sharing JSON scalar node of the same leaf value
 * regardless of its parent key.
 * This makes resulting OSON size as compact/small as possible. But this
 * encoding mode is very expensive. So it is seldomly used.
 * 
 * [ {"a": "finished", "b": "finished" }, {"b": "finished"}]
 * leaf node "finished" is shared in oson image.
 */

/* share all possible leaf scalar nodes having the same leaf value*/
#define JZNDOM_OCT_SHR_ALL_SCALAR          0x04000000

/* offsets in array or object are relative to its parent during OSON
 * encoding so that the resulting OSON is friendly for compression
 * Example:
 * Normal oson encoding:
 * array node is at offset=100, its array content is [110, 140, 160].
 * array node is at offset=220, its array content is [230, 260, 300].
 *
 * When JZNDOM_OCT_STR_REL_OFFSET is set,
 * we encode the first array  as  [10, 40, 60].
 * we encode the second array  as [10, 40, 80].
 *
 *
 * So compression algorithm can now compress 10, 40.
 */

#define JZNDOM_OCT_STR_REL_OFFSET          0x08000000

/* all shared nodes via JZNDOM_OCT_SHR_SIMP_NODES and
 * JZNDOM_OCT_SHR_PA_SCALAR are clustered together in a 
 * a shared node cluster segement within the oson image.
 * In this way, it is easy to identify if the node is shared for
 * ease of direct node content replacement for partial oson update
 */
#define JZNDOM_OCT_COMPACT_CLUSTER         0x10000000

/* always use ub4 to store tree size, needed for template oson tree */
#define JZNDOM_OCT_TREE_SZ_UB4             0x20000000
/* OSON dom created not for updating OSON */
#define JZNDOM_OCT_NOT_4_UPD               0x40000000
/*
** By default, all JSON number are stored as ORA number format.
** However, for internal testing purpose, this JZNDOM_NUM_AS_DB stores
** JSON number as binary double (Oracle big-endian based binary double)
**
** ### DEPRECATED - See JZNDOM_NUM_AS_ORA
*/
#define JZNDOM_NUM_AS_DB                   0x80000000

/* Bug37204122: dom api to push down range cmp/in-clause */
typedef struct jznEng_RngCmpInf jznEng_RngCmpInf;

typedef struct jznEng_InClauseInfo jznEng_InClauseInfo;

struct jznpFunction;

/*
** NAME:
**   jznDomAPI
**
** DESCRIPTION:
**   This is really Java Interface or C++ Pure abstract class.
**   DOM API is a set of C function pointers; this provides an abstract
**   interface whose concrete implementations are defined by each JSON
**   DOM implementors. Callers are expected to use the macros to invoke
**   the JSON DOM "methods"; these macros dereference the function pointers
**   and call them, passing the instance as an argument.
*/
struct jznDomAPI
{
  /*
  ** Loader Methods
  */

  jznDomNode *(*jznDomLoadFromInputEventSrc)(jznDomDoc *jdoc,
                                             jznEventSource *evtsrc);

  jznDomNode *(*jznDomLoadFromInputOSON)(jznDomDoc *jdoc,
                                         jznosonFile *octbsrc);

  /*
  ** Reader Methods
  */

  jznnodetype (*jznDomGetNodeType)(jznDomDoc *jdoc, jznDomNode *node);

  jsonerr     (*jznDomGetScalarInfo)(jznDomDoc *jdoc, jznDomScalar *nd,
                                     jznScalarVal *val /* OUT */);

  jznDomNode *(*jznDomGetRootNode)(jznDomDoc *jdoc);

  ub4         (*jznDomGetNumObjField)(jznDomDoc *jdoc, jznDomObject *obj);

  jznDomNode *(*jznDomGetFieldVal)(jznDomDoc *jdoc,
                                   jznDomObject *obj,
                                   jznDomObjFieldName *nmkey);

  jznDomNode *(*jznDomGetFieldByName)(jznDomDoc *jdoc, jznDomObject *obj,
                                      oratext *nm, ub2 nmlen);

  void (*jznDomGetAllFieldNamesAndVals)(jznDomDoc *jdoc, jznDomObject *obj,
                                        jznDomNameValuePair nvps[] /* OUT */);

  ub4 (*jznDomGetFieldNamesAndValsBatch)(jznDomDoc *jdoc, jznDomObject *obj,
                                         ub4 startPos, ub4 fetchSz,
                                         jznDomNameValuePair nvps[] /* OUT */);

  ub4         (*jznDomGetArraySize)(jznDomDoc *jdoc, jznDomArray *ary);

  jznDomNode *(*jznDomGetArrayElem)(jznDomDoc *jdoc,
                                    jznDomArray *ary, ub4 index);

  ub4         (*jznDomGetArrayElemBatch)(jznDomDoc *jdoc, jznDomArray *ary,
                                         ub4 startPos, ub4 fetchSz,
                                         jznDomNode *ndary[] /* OUT */);

  /*
  ** Mutator Methods
  */

  jsonerr (*jznDomSetRootNode)(jznDomDoc *jdoc, jznDomNode *root);

  jsonerr (*jznDomPutFieldValue)(jznDomDoc *jdoc, jznDomObject *obj,
                                 oratext *name, ub2 namelen, jznDomNode *node);

  boolean (*jznDomPutItem)(jznDomDoc *jdoc, jznDomArray *arr,
                           jznDomNode *node, ub4 pos);

  boolean (*jznDomAppendItem)(jznDomDoc *jdoc,
                              jznDomArray *arr, jznDomNode *node);

  boolean (*jznDomReplaceItem)(jznDomDoc *jdoc, jznDomArray *arr,
                               jznDomNode *node, ub4 pos);

  boolean (*jznDomDeleteField)(jznDomDoc *jdoc, jznDomObject *obj,
                               jznDomObjFieldName *nmkey);

  jznDomNode *(*jznDomUnlinkField)(jznDomDoc *jdoc, jznDomObject *obj,
                                   jznDomObjFieldName *nmkey);

  jsonerr     (*jznDomRenameField)(jznDomDoc *jdoc, jznDomObject *obj,
                                   oratext *oldName, ub2 oldNameLen,
                                   oratext *newName, ub2 newNameLen);

  boolean     (*jznDomDeleteItem)(jznDomDoc *jdoc, jznDomArray *arr, ub4 idx);

  jznDomNode *(*jznDomUnlinkItem)(jznDomDoc *jdoc, jznDomArray *arr, ub4 idx);

  ub4         (*jznDomDeleteItemBatch)(jznDomDoc *jdoc, jznDomArray *arr,
                                       ub4 start, ub4 deleteSz);

  /*
  ** Constructor Methods (including destroy/free/reset)
  */

  jznDomObject *(*jznDomNewObject)(jznDomDoc *jdoc, ub4 sz);

  jznDomArray  *(*jznDomNewArray)(jznDomDoc *jdoc, ub4 sz);

  jznDomScalar *(*jznDomNewScalar)(jznDomDoc *jdoc, jznScalarVal *val);

  void          (*jznDomDocReset)(jznDomDoc *jdoc);

  void          (*jznDomDocFree)(jznDomDoc *jdoc);

  /*
  ** Utility Methods
  */

  jznEventSource *(*jznDomGetOutputEventSrc)(jznDomDoc *jdoc);

  /* Checks whether two given JSON documents are equal.
  ** One should check the error status by calling JsonDomGetError().
  */
  boolean     (*jznDomEquals)(jznDomDoc *jdoc1, jznDomNode *nd1,
                              jznDomDoc *jdoc2, jznDomNode *nd2);

  jznDomNode *(*jznDomCopy)(jznDomDoc *srcdoc, jznDomNode *srcnode,
                            jznDomDoc *destdoc);

  jsonerr (*jznDomValidFid)(jznDomDoc *jdoc,
                            jznDomObjFieldName *fnms, ub2 fnmsn);

  jsonerr (*jznDomStoreField)(jznDomDoc *jdoc, oratext *fname, ub4 fnlen,
                              jznDomObjFieldName *name /* OUT */);

  jsonerr (*jznDomPrintNode)(jznDomDoc *jdoc, jznDomNode *node,
                             jznEventWriter *writer); /* printing method */

  void    (*jznDomNodeVisitor)(jznDomDoc *jdoc, jznDomNode *node,
                               JsonDomVisitorFunc func, void *ctx);
                                                             /* visitor Func */

  /*
  ** Additional Methods
  */

  jznDomScalar *(*jznDomNewScalarVal)(jznDomDoc *jdoc, jznvaltype typ, ...);

  boolean  (*jznDomDeleteFieldByName)(jznDomDoc *jdoc, jznDomObject *obj,
                                      oratext *name, ub2 namelen);

  jznDomNode *(*jznDomUnlinkFieldByName)(jznDomDoc *jdoc, jznDomObject *obj,
                                         oratext *name, ub2 namelen);

  boolean (*jznDomFreeNode)(jznDomDoc *jdoc, jznDomNode *node);

  jsonerr (*jznDomGetScalarInfoOci)(jznDomDoc *jdoc, jznDomScalar *nd,
                                    jznScalarVal *val /* OUT */,
                                    JsonOCIVal   *aux /* OUT */);

  oratext *(*jznDomGetError)(jznDomDoc *jdoc,
                             jsonerr *err /* OUT */, boolean clear);

  jznDomNode *(*jznDomReplaceRootNode)(jznDomDoc *jdoc, jznDomNode *root);

  jznDomNode *(*jznDomImportFromEventSrc)(jznDomDoc *jdoc,
                                          jznEventSource *evtsrc,
                                          boolean readOneItem);

  jznDomNode *(*jznDomGetParent)(jznDomDoc *jdoc, jznDomNode *node);

  jsonerr (*jznDomSetNodeProperties)(jznDomDoc *jdoc, jznDomNode *node, ...);

  jznDomNode *(*jznDomLoad)(jznDomDoc *jdoc, jsonerr  *jerr, ...);

  jsonerr  (*jznDomSerialize)(jznDomDoc *jdoc, ...);

  jznDomNode *(*jznDomGetFieldPos)(jznDomDoc *jdoc, jznDomObject *obj,
                                   oratext *nm, ub2 nmlen, ub4 *pos /* OUT */);

  ub4      (*jznDomUnlinkElems)(jznDomDoc *jdoc, jznDomArray *arr,
                                ub4 pos, ub4 numNodes, jznDomNode **nodes);

  ub4      (*jznDomInsertItems)(jznDomDoc *jdoc, jznDomArray *arr,
                                ub4 pos, ub4 numNodes, jznDomNode **nodes);

  // ### Consider dropping this API
  jsonerr  (*jznDomSortArray)(jznDomDoc *jdoc, jznDomArray *arr, uword flags);

  jsonerr (*jznDomCastValue)(jznDomDoc *jdoc, jznDomScalar *snode,
                             jznExtnType styp);

  jznDomScalar *(*jznDomDupScalar)(jznDomDoc *jdoc, jznDomScalar *snode);

  jznDomScalar *(*jznDomSetScalar)(jznDomDoc *jdoc, jznDomScalar *snode,
                                   jznScalarVal *val);

  jznDomNode *(*jznDomReplaceField)(jznDomDoc *jdoc, jznDomObject *obj,
                                    oratext *nm, ub2 nmlen, jznDomNode *node);

  /* reset method with input flag parameter */
  void          (*jznDomDocResetWF)(jznDomDoc *jdoc, ub4 flag);

  jznDomNode * (*jznDomDocDecodeOSONBytesIntoNode)(jznDomDoc *jdoc,
                  ub1 *oson_byte, ub4 oson_byte_len, jsonerr *ojerr,
                  jznDomDoc **out_jdoc);

  /* Bug37204122: dom api to push down range cmp/in-clause */
  /* evaluate range comp filter over array of elements to see if any array
   * element of scalar value satisifies the filter
   */
  boolean (*jznDomAryFilter)(jznDomDoc *jdoc, jznDomArray *jarray_node,
                             jznEng_RngCmpInf *cmpI);

  /* evaluate range comp filter over the scalar node to see if the scalar
   * node of the scalar value satisifies the filter
   */
  boolean (*jznDomScalarFilter)(jznDomDoc *jdoc, jznDomScalar *jscalar_node,
                                jznEng_RngCmpInf *cmpI);

  /* evaluate in clause whose value is sorted using pre-sorted merge join as the
   * jarray_node of scalar values is sorted. The inclause must be type(strict)
   */
  boolean (*jznDomAryInClauseMrgSort)(jznDomDoc *jdoc, jznDomArray *jarray_node,
                                      jznEng_InClauseInfo *incI, boolean *rslt);

  /* evaluate in clause whose value is sorted using binary search, the
   * jarray_node of scalar values may NOT be sorted */
  boolean (*jznDomAryInClauseBinSrch)(jznDomDoc *jdoc, jznDomArray *jarray_node,
                                      jznEng_InClauseInfo *incI, boolean *rslt);

  /* given a json array, it finds the min or max of the element node using
   * json datatype range comparison (DeepDomNodesCompare) semantics
   */
  jznDomNode * (*jznDomAryMinMaxJt)(jznDomDoc *jdoc, jznDomArray *jarray_node,
                                    struct jznpFunction  *fdesc);

  /* given a json array, it applies the aggregation func, such as minNumer(),
   * maxNumber(), minTime(), maxTime() that returns a scalar value
   * capture via (jtype_o, valptr_o, vallen_o)
   * Caller is expected to set valptr_o to point to memory bigger than 
   * *vallen_o
   */
  jsonerr (*jznDomAryAggRetSca)(jznDomDoc *jdoc, jznDomArray *jarray_node,
            struct jznpFunction  *fdesc, ub4 modeFlags, ub4 maxlen,
            jznvaltype* jtype_o, ub1 *valptr_o, ub4 * vallen_o);
};


/*---------------------------------------------------------------------------
                                PUBLIC FUNCTIONS
  ---------------------------------------------------------------------------*/

/*------------------------- DOM Constructor interfaces ----------------------*/

/*
** NAME:
**   JsonDomCreate
**
** PARAMETERS:
**   xmlctx      *xctx     (IN)  - XML Context
**   jsonerr     *jerr     (OUT) - JSON Error Code (returned)
**
** RETURNS:
**   (JsonDomDoc *) JSON DOM Document Container.
**   Returns a NULL on an error condition (e.g. out of memory).
**
** DESCRIPTION:
**   Constructor function for JSON DOM Container (JsonDomDoc *). Valid flags
**   for JZN_MODE are defined above (See JSON DOM Constructor flags).
**
** NOTES:
**   Supported varargs parameters:
**   ===================================
**   Property Name       Expected Type
**   ===================================
**   JZN_FILE            oratext *
**   JZN_BUFFER          void *
**   JZN_BUFFERLEN       ub4
**   JZN_STREAM          orastream *
**   JZN_FORMAT          oratext * ("JSON" or "OSON")
**   JZN_MEMCTX          oramemctx *
**   JZN_FATAL_HANDLER   void (*jznFatalErrHandler)(xmlctx *, oratext *)
**   JZN_MODE            ub4
**
*/
JsonDomDoc  *JsonDomCreate(xmlctx  *xctx, jsonerr  *jerr,  ...);

JsonDomDoc  *JsonDomCreateVA(xmlctx  *xctx, jsonerr  *jerr,  va_list va);

/*
** This function returns a concrete implementation class that inherits
** jznDomDoc as the base class, which implements jznDomAPI interface.
**
** Returns a NULL on an error in parameters, or out of memory. Valie flags
** are defined above (See JSON DOM Constructor flags).
**
** ### This is an outdated interface.
** ### The JsonDomCreate() interface has an error return argument.
*/
jznDomDoc *jznCreateDom(xmlctx *xctx, jznDomTyp typ,
                        jznFatalErrHandler eh, ub4 flag, ...);

jznDomDoc *jznCreateDomVA(xmlctx *xctx, jznDomTyp typ,
                          jznFatalErrHandler eh, ub4 flag, va_list va);


/*------------------------------ Loader interfaces --------------------------*/

/*
** NAME:
**   JsonLoadDom
**
** PARAMETERS:
**   jznDomDoc    *jdoc    (IN)  - document to load into
**   jsonerr       jerr    (OUT) - output JSON error code
**   list                  (IN)  - NULL-terminated list of variable arguments
**
**   Supported varargs parameters:
**   =======================================================================
**   Property Name       Expected Type     Valid values      Notes
**   =======================================================================
**   JZN_FILE            oratext *
**   JZN_BUFFER          void *
**   JZN_BUFFERLEN       ub4
**   JZN_STREAM          orastream *
**   JZN_FORMAT          oratext *         "JSON", "OSON"
**   JZN_ENCODING        ub2                                 For "JSON" only
**   JZN_ALLOW           ub4                                 Parse flags for
**                                                           "JSON" only,
**                                                           JZN_RUN_BULK_LOAD
**                                                           for "OSON" allowed
**
** RETURNS:
**   (jznDomNode *) the root node of the DOM, or NULL on error.
**
** DESCRIPTION:
**   Ingests the data supplied in buffer/stream/file as a tree of nodes into
**   the target <jdoc>. Sets the new tree of nodes as the root node of <jdoc>,
**   and frees any previous root node.
**   This is typically used after constructing an empty jznDomDoc to load
**   it from a buffer/stream/file. The source data can either be textual
**   JSON (JZN_FORMAT = "JSON") or binary OSON (JZN_FORMAT = "OSON").
**
**   If you load into a DOM that is not empty, this operation will free
**   the old root node and all decendants, plus all orphan nodes.
**
** NOTES:
**   If source data is OSON, the target DOM should be JZN_OSON_DOM type.
**   If source data is text, the target DOM should be JZN_INMEM_DOM type.
*/
#define JsonLoadDom(jdoc, jerr, ...) \
  ((jdoc)->mthds_jznDomDoc->jznDomLoad((jdoc), (jerr), __VA_ARGS__))

/*
** NAME:
**   JsonDomLoadInput
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN)  - document to load events into
**   jznEventSource *evtsrc  (IN)  - event source (such as the JSON parser)
**
** RETURNS:
**   (jznDomNode *) the root node of the DOM, or NULL on error.
**
** DESCRIPTION:
**   Ingests the event source as a tree of nodes into the target <jdoc>.
**   Sets the new tree of nodes as the root node of <jdoc>, and frees any
**   previous root node.
**   This is typically used after constructing an empty jznDomDoc to load
**   it from a JSON event stream such as the JSON parser or BSON decoder.
**
** NOTES:
**   Always check the error code by calling JsonDomGetError before proceeding.
**
**   If you load into a DOM that is not empty, this operation will free
**   the old root node and all decendants, as per JsonDomSetRoot.
*/
#define JsonDomLoadInput(jdoc, evtsrc) \
  ((jdoc)->mthds_jznDomDoc->jznDomLoadFromInputEventSrc((jdoc), (evtsrc)))

/*
** NAME:
**   JsonDomLoadImage
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - document to map OSON image into
**   jznosonFile    *octbsrc (IN) - OSON image to load
**
** RETURNS:
**   (jznDomNode *) the root node of the DOM, or NULL on error.
**
** DESCRIPTION:
**   This method is currently only supported for JZN_OSON_DOM type DOMs.
**   After constructing an empty jznDomDoc, it can be mapped to an OSON
**   serialized image using this method. The resulting DOM is read-only
**   and operations are backed directly by the OSON image.
**
** NOTES:
**   Always check the error code by calling JsonDomGetError before proceeding.
**
** ### In the future, this method can also be supported for
** ### JZN_INMEM_DOM type of jznDomDoc to load a mutable in-memory DOM
** ### from persistent OSON storage, necessary to allow updates.
*/
#define JsonDomLoadImage(jdoc, octbsrc) \
  ((jdoc)->mthds_jznDomDoc->jznDomLoadFromInputOSON((jdoc), (octbsrc)))

/*
** NAME:
**   JsonDomImportEvents
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN)  - document to load events into
**   jznEventSource *evtsrc  (IN)  - event source (such as the JSON parser)
**
** RETURNS:
**   (jznDomNode *) the new orphan node imported, or NULL on error.
**
** DESCRIPTION:
**   Ingests the event source as a tree of nodes into the target <jdoc>.
**   Returns the top node of the imported tree, which is an orphan node.
**
** NOTES:
**   Always check the error code by calling JsonDomGetError before proceeding.
*/
#define JsonDomImportEvents(jdoc, evtsrc) \
  ((jdoc)->mthds_jznDomDoc->jznDomImportFromEventSrc((jdoc), (evtsrc), FALSE))

/*
** NAME:
**   JsonDomLoadItem
**
** PARAMETERS:
**   JsonDomDoc      *jdoc    (IN)  - document to load events into
**   JsonEventSource *evtsrc  (IN)  - event source (such as the JSON parser)
**
** RETURNS:
**   (JsonDomNode *) orphan node representing the item loaded to the DOM
**
** DESCRIPTION:
**   Loads the next item from an event source to the target DOM, returning
**   it as an orphan node. If the item is a container, all events within
**   the container are loaded until the container is closed.
**
** NOTES:
**   This is designed to allow part of an in-progress event stream to be
**   directed into a DOM, followed by the resumption of consuming the
**   event stream by the caller. If an entire event source is put to this
**   method, the JZNEV_END event is not consumed.
*/
#define JsonDomLoadItem(jdoc, evtsrc) \
  ((jdoc)->mthds_jznDomDoc->jznDomImportFromEventSrc((jdoc), (evtsrc), TRUE))


/*------------------------------ Read interfaces ----------------------------*/

/*
** NAME:
**   JsonDomGetNodeType
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**   jznDomNode     *node    (IN) - node within the DOM
**
** RETURNS:
**   (jznnodetype) the type of node (Object, Array, Scalar)
**
** DESCRIPTION:
**   Used to test the type of a node. The node may be safely cast to
**   the subclass type, e.g. if the return is JZNDOM_SCALAR, the node
**   may be cast to (jznDomScalar *).
**
** ERROR HANDLING: It may return JZNDOM_SCALAR in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
**
** Caller must call JsonDomGetError(jdoc) after call of JsonDomGetNodeType()
** to check for error first.
*/
#define JsonDomGetNodeType(jdoc, node) \
   ((jdoc)->mthds_jznDomDoc->jznDomGetNodeType((jdoc), (node)))

/*
** NAME:
**   JsonDomGetScalarInfo
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN)  - JSON DOM
**   jznDomScalar   *nd      (IN)  - scalar node to get information about
**   jznScalarVal   *val     (OUT) - returns information about the scalar
**
** RETURNS:
**   (jsonerr)  error code, JZNERR_OK (0) for no error
**
** DESCRIPTION:
**   Used to retrieve a scalar value, which is returned in the val structure
**   (which must be supplied by the caller, often a pointer to a local
**   variable in the calling code). The return structure is a discriminated
**   union. The value's scalar subtype (e.g. JZNVAL_NUMBER) is available
**   as the first member, and the scalar's actual value can be read from
**   the appropriate union member.
**
** NOTES:
**   Variable-length types such as strings, JSON numbers, and Oracle
**   internal data types, are returned as pointers to the underlying
**   DOM data image or node. Small fixed-length types such as signed
**   integers and doubles are returned directly in the union.
**
** ERROR HANDLING: It may return error code in case of error with errrcode
** obtaineable via JsonDomGetError(srcdoc)
*/
#define JsonDomGetScalarInfo(jdoc, nd, val) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetScalarInfo((jdoc), (nd), (val)))

/*
** NAME:
**   JsonDomGetScalarInfoOCI
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN)  - JSON DOM
**   jznDomScalar   *nd      (IN)  - scalar node to get information about
**   jznScalarVal   *val     (OUT) - returns information about the scalar
**
** RETURNS:
**   (jsonerr)  error code, JZNERR_OK (0) for no error
**
** DESCRIPTION:
**   Used to retrieve a scalar value, which is returned in the val structure
**   (which must be supplied by the caller, often a pointer to a local
**   variable in the calling code). The return structure is a discriminated
**   union. The value's scalar subtype (e.g. JZNVAL_NUMBER) is available
**   as the first member, and the scalar's actual value can be read from
**   the appropriate union member.
**
** NOTES:
**   Oracle native types are unpickled and returned in structured form in
**   auxiliary union of type JsonOCIVal*.
**
** ERROR HANDLING: It may return error code in case of error with errrcode
** obtaineable via JsonDomGetError(srcdoc)
*/
#define JsonDomGetScalarInfoOci(jdoc, nd, val, aux) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetScalarInfoOci((jdoc), (nd), (val), (aux)))

/*
** NAME:
**   JsonDomGetRoot
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**
** RETURNS:
**   (jznDomNode *) root node for the DOM (or NULL if none is set)
**
** DESCRIPTION:
**   Returns the root node of a DOM, which is one of the three types of
**   nodes (although rare, a scalar with no children is a valid JSON
**   document per the RFC).
**
** NOTES:
**   So-called "orphan" nodes can't be accessed by descending the tree
**   starting from the root.
**
** ERROR HANDLING: It may return error code in case of error with errrcode
** obtaineable via JsonDomGetError(srcdoc)
*/
#define JsonDomGetRoot(jdoc) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetRootNode((jdoc)))

/*
** NAME:
**   JsonDomGetNumFields
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**   jznDomObject   *obj     (IN) - Object node within the DOM
**
** RETURNS:
**   (ub4) a count of the number of fields within this object.
**
** DESCRIPTION:
**   Returns the number of fields within the object.
**
** NOTES:
**   Duplicate field names are not allowed in most DOMs.
**
** ERROR HANDLING: It may return 0 in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetNumFields(jdoc, obj) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetNumObjField((jdoc), (obj)))

/*
** NAME:
**   JsonDomGetFieldValue
**
** PARAMETERS:
**   jznDomDoc          *jdoc  (IN) - JSON DOM
**   jznDomObject       *obj   (IN) - Object node within the DOM
**   jznDomObjFieldName *nmkey (IN) - Field key structure
**
** RETURNS:
**   (jznDomNode *) the value of the field (may be any JSON node type)
**
** DESCRIPTION:
**   Returns the value of a field within the object, retrieving it by matching
**   the key structure. The key structure contains a pointer to the field
**   name and the length of the field name, as well as optional hash and id
**   codes that may speed retrieval when using an OSON based DOM.
**   Returns NULL if a field matching the key cannot be found.
**
** NOTES:
**   For JZNOCT implemented DOM, nm->hashid_jznDomObjFieldName field
**   must be set to quickly identify the field with that name. Set it
**   to 0 if it's unknown.
**   Also, nm->id_jznDomObjFieldName must be 0, unless the <nmkey>
**   structure was initialized specifically for the current document.
**   See JsonDomCheckFieldKeys().
**
** ERROR HANDLING: It may return NULL in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetFieldValue(jdoc, obj, nmkey) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetFieldVal((jdoc), (obj), (nmkey)))

/*
** NAME:
**   JsonDomGetFieldByName
**
** PARAMETERS:
**   jznDomDoc       *jdoc   (IN) - JSON DOM
**   jznDomObject    *obj    (IN) - Object node within the DOM
**   oratext         *fname  (IN) - Field name string
**   ub2              fnlen  (IN) - Length of field name string in bytes
**
** RETURNS:
**   (jznDomNode *) the value of the field (may be any JSON node type)
**
** DESCRIPTION:
**   Returns the value of a field within the object, retrieving it by matching
**   the name string and length.
**   Returns NULL if a matching field cannot be found.
**
** NOTES:
**   A convenience interface that is otherwise identical in function
**   to JsonDomGetFieldValue.
**
** ERROR HANDLING: It may return NULL in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetFieldByName(jdoc, obj, fname, fnlen) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetFieldByName((jdoc),(obj),(fname),(fnlen)))

/*
** NAME:
**   JsonDomGetAllFields
**
** PARAMETERS:
**   jznDomDoc           *jdoc   (IN)  - JSON DOM
**   jznDomObject        *obj    (IN)  - Object node within the DOM
**   jznDomNameValuePair  nvps[] (OUT) - Returns all field name/value pairs
**
** RETURNS:
**   (void)
**
** DESCRIPTION:
**   Returns all fields for an object by populating an array of name/value
**   pairs. The name portion is a jznDomObjFieldName (the key structure
**   as used in JsonDomGetFieldValue). The value portion is simply a node
**   pointer.
**
** NOTES:
**   The array is assumed to be large enough to hold the result. The caller
**   takes responsibility for this, calling JsonDomGetNumFields and if
**   necessary allocating an array of sufficient size.
**
** ERROR HANDLING: It may return in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetAllFields(jdoc, obj, nvps) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetAllFieldNamesAndVals((jdoc), (obj), \
                                                          (nvps)))

/*
** NAME:
**   JsonDomGetFields
**
** PARAMETERS:
**   jznDomDoc           *jdoc     (IN)  - JSON DOM
**   jznDomObject        *obj      (IN)  - Object node within the DOM
**   ub4                  startPos (IN)  - Starting position (0-based)
**   ub4                  fetchSz  (IN)  - Number of fields to retrieve
**   jznDomNameValuePair  nvps[]   (OUT) - Returns field name/value pairs
**
** RETURNS:
**   (ub4) number of fields returned in the output name/value array
**
** DESCRIPTION:
**   Returns a set of fields in a batch interface. This allows a caller
**   to use a fixed-sized array to iterate over the fields, while still
**   retrieving them in bulk. The fields are returned in the same output
**   format as JsonDomGetAllFields.
**
** NOTES:
**   The first field is at start position 0. The iteration is backed directly
**   by the DOM object, so this interface won't work properly if the object
**   undergoes modification during the iteration.
**
** ERROR HANDLING: It may return 0 in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetFields(jdoc, obj, startPos, fetchSz, nvps) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetFieldNamesAndValsBatch((jdoc), (obj), \
                                                (startPos), (fetchSz), (nvps)))

/*
** NAME:
**   JsonDomGetArraySize
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**   jznDomArray    *ary     (IN) - Array node within the DOM
**
** RETURNS:
**   (ub4) a count of the number of elements within this array.
**
** DESCRIPTION:
**   Returns the number of elements within the array.
**
** NOTES:
**   Duplicate field names are not allowed in most DOMs.
**
** ERROR HANDLING: It may return 0 in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetArraySize(jdoc, ary) \
   ((jdoc)->mthds_jznDomDoc->jznDomGetArraySize((jdoc), (ary)))

/*
** NAME:
**   JsonDomGetArrayElement
**
** PARAMETERS:
**   jznDomDoc       *jdoc   (IN) - JSON DOM
**   jznDomArray     *ary    (IN) - Object node within the DOM
**   ub4              index  (IN) - Position within the array (0-based)
**
** RETURNS:
**   (jznDomNode *) the value of the array item (may be any JSON node type)
**
** DESCRIPTION:
**   Returns the node at a specific array index, starting from position 0.
**   if the index is out of bounds, a NULL is returned.
**
** NOTES:
**   An error code may be set if the index is out of bounds. The caller
**   should check and if necessary clear the error. (For example, if a
**   caller doesn't bother using JsonDomGetArraySize and iterates the array
**   while relying on the NULL to signal end-of-iteration.)
**
** ERROR HANDLING: It may return NULL in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetArrayElement(jdoc, ary, index) \
   ((jdoc)->mthds_jznDomDoc->jznDomGetArrayElem((jdoc), (ary), (index)))

/*
** NAME:
**   JsonDomGetElements
**
** PARAMETERS:
**   jznDomDoc           *jdoc     (IN)  - JSON DOM
**   jznDomArray         *ary      (IN)  - Array node within the DOM
**   ub4                  startPos (IN)  - Starting position (0-based)
**   ub4                  fetchSz  (IN)  - Number of fields to retrieve
**   jznDomNode          *ndary[]  (OUT) - Returns nodes from the array
**
** RETURNS:
**   (ub4) number of elements returned in the output node array
**
** DESCRIPTION:
**   Returns a set of elements in a batch interface. This allows a caller
**   to use a fixed-sized array to iterate over the elements, while still
**   retrieving them in bulk.
**
** NOTES:
**   The first element at start position 0. The iteration is backed directly
**   by the DOM array, so this interface won't work properly if the array
**   undergoes modification during the iteration.
**
** ERROR HANDLING: It may return 0 in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetElements(jdoc, ary, startPos, fetchSz, ndary) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetArrayElemBatch((jdoc), (ary), \
                                        (startPos), (fetchSz), (ndary)))


/*---------------------------- Mutator interfaces ---------------------------*/

/*
** NAME:
**   JsonDomSetRoot
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomNode    *root     (IN)  - orphan node within the DOM
**
** RETURNS:
**   (jsonerr)
**
** DESCRIPTION:
**   Sets the specified node as the root. The new node may be a NULL.
**   The new root must be an orphan node - that is, it cannot be a node
**   that exists within the tree descending from the old root, if any.
**   Orphan nodes are created by constructors, or by copying node(s)
**   from another DOM document, or by unlinking them from the DOM tree.
**
** NOTES:
**   The old root and all descendants is freed.
*/
#define JsonDomSetRoot(jdoc, root) \
  ((jdoc)->mthds_jznDomDoc->jznDomSetRootNode((jdoc), (root)))

/*
** NAME:
**   JsonDomReplaceRoot
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomNode    *root     (IN)  - orphan node within the DOM
**
** RETURNS:
**   (jznDomNode *)
**
** DESCRIPTION:
**   Sets the specified node as the root. The new node may be a NULL.
**   The new root must be an orphan node - that is, it cannot be a node
**   that exists within the tree descending from the old root, if any.
**   Orphan nodes are created by constructors, or by copying node(s)
**   from another DOM document, or by unlinking them from the DOM tree.
**
** NOTES:
**   Returns the old root node, if any, after unlinking it.
*/
#define JsonDomReplaceRoot(jdoc, root) \
  ((jdoc)->mthds_jznDomDoc->jznDomReplaceRootNode((jdoc), (root)))

/*
** NAME:
**   JsonDomGetParent
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomNode    *node     (IN)  - node within the DOM
**
** RETURNS:
**   (jznDomNode *)
**
** DESCRIPTION:
**   Get the parent node of a specified node. This returns NULL for an
**   orphan node or on any error. The parent may be unavailable for some
**   DOMs, for example on a binary image, or if the node has multiple
**   parents.
**
** NOTES:
**   If an error is possible, the caller needs to call JsonDomGetError.
**   on a NULL return. Possible errors:
**   - node was previous freed
**   - node has multiple parents
**   - method is unimplemented in this DOM
*/
#define JsonDomGetParent(jdoc, node) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetParent((jdoc), (node)))

#define JZNDOM_NODE_REF "reference"
/*
** NAME:
**   JsonDomRefNode
**   JsonDomDerefNode
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomNode    *node     (IN)  - node within the DOM
**
** RETURNS:
**   (jsonerr)
**
** DESCRIPTION:
**   Mark a node as externally referenced or dereferenced. A node marked
**   as referenced is immune from free operations. A node not marked as
**   referenced may be freed if it has no outstanding parent node in the DOM.
**
** NOTES:
**   May return an error if the underlying DOM can't support the property.
**   This API is intended only for internal implementations such as
**   the PL/SQL DOM, where it's necessary to manage external references
**   from the host language via this mechanism.
*/
#define JsonDomRefNode(jdoc, node) \
  ((jdoc)->mthds_jznDomDoc->jznDomSetNodeProperties((jdoc), (node), \
                                         JZNDOM_NODE_REF, TRUE, NULL)
#define JsonDomDerefNode(jdoc, node) \
  ((jdoc)->mthds_jznDomDoc->jznDomSetNodeProperties((jdoc), (node), \
                                         JZNDOM_NODE_REF, FALSE, NULL)

/*
** NAME:
**   JsonDomSetField
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomObject  *obj      (IN)  - object within the DOM
**   oratext       *name     (IN)  - string with field name to create/replace
**   ub2            namelen  (IN)  - length of field name string
**   jznDomNode    *node     (IN)  - orphan node to set as field value
**
** RETURNS:
**   (jsonerr)  error code, JZNERR_OK (0) for no error
**
** DESCRIPTION:
**   Sets the value of the specified field to the specified orphan node.
**   The field is created if it doesn't exist, otherwise the old value is
**   replaced.
**
** NOTES:
**   The old field value, if any, is freed along with all descendants.
*/
#define JsonDomSetField(jdoc, obj, name, namelen, node) \
  ((jdoc)->mthds_jznDomDoc->jznDomPutFieldValue((jdoc), (obj), \
                                                (name), (namelen), (node)))

/*
** NAME:
**   JsonDomReplaceField
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomObject  *obj      (IN)  - object within the DOM
**   oratext       *name     (IN)  - string with field name to replace
**   ub2            namelen  (IN)  - length of field name string
**   jznDomNode    *node     (IN)  - orphan node to set as field value
**
** RETURNS:
**   (jznDomNode *) old field value
**
** DESCRIPTION:
**   Sets the value of the specified field to the specified orphan node.
**   Unlinks and returns the old value. Returns NULL if none.
**
** NOTES:
**   An error also returns NULL - the caller must check JsonDomGetError.
**
**   This is equivalent to UnlinkField followed by SetField. However,
**   that combination may cause the field's "position" to change when
**   serialized, while this function will not. A similar problem doesn't
**   exist for arrays, where UnlinkElement followed by AddElement is
**   always positional.
*/
#define JsonDomReplaceField(jdoc, obj, name, namelen, node) \
  ((jdoc)->mthds_jznDomDoc->jznDomReplaceField((jdoc), (obj), \
                                                (name), (namelen), (node)))

/*
** NAME:
**   JsonDomAddElement
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   jznDomNode    *node     (IN)  - orphan node to set as element value
**   ub4            pos      (IN)  - position within the array (0-based)
**
** RETURNS:
**   (boolean)  TRUE on success, FALSE on error
**
** DESCRIPTION:
**   Inserts an orphan node as a new element in an array. The insertion
**   is done at the position specified, with 0 meaning that the inserted
**   node becomes the first element in the array, and all other elements
**   are shifted, increasing the size of the array by 1 element. The
**   array size bound may be specified as a means of appending a node
**   to the end of the array.
**
** ERROR HANDLING: It may return FALSE in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomAddElement(jdoc, arr, node, pos) \
  ((jdoc)->mthds_jznDomDoc->jznDomPutItem((jdoc), (arr), (node), (pos)))

/*
** NAME:
**   JsonDomAppendElement
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   jznDomNode    *node     (IN)  - orphan node to set as element value
**
** RETURNS:
**   (boolean)  TRUE on success, FALSE on error
**
** DESCRIPTION:
**   Append an orphan node as a new element in an array, at the last
**   position within the array. Equivalent to calling JsonDomAddElement
**   and giving the array size as the insertion position.
**
** ERROR HANDLING: It may return FALSE in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomAppendElement(jdoc, arr, node) \
  ((jdoc)->mthds_jznDomDoc->jznDomAppendItem((jdoc), (arr), (node)))

/*
** NAME:
**   JsonDomReplaceElement
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   jznDomNode    *node     (IN)  - orphan node to set as element value
**   ub4            pos      (IN)  - position within the array (0-based)
**
** RETURNS:
**   (boolean)  TRUE on success, FALSE on error
**
** DESCRIPTION:
**   Sets the value of an array element to the specified orphan node.
**   The position given must be within the array bounds.
**
** NOTES:
**   The old element node is freed along with all descendants.
**
** ERROR HANDLING: It may return FALSE in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomReplaceElement(jdoc, arr, node, pos) \
  ((jdoc)->mthds_jznDomDoc->jznDomReplaceItem((jdoc), (arr), (node), (pos)))

/*
** NAME:
**   JsonDomDeleteField
**
** PARAMETERS:
**   jznDomDoc          *jdoc  (IN)  - JSON DOM
**   jznDomObject       *obj   (IN)  - object within the DOM
**   jznDomObjFieldName *nmkey (IN)  - Field key structure
**
** RETURNS:
**   (boolean)  TRUE on success, FALSE if the field was not found
**
** DESCRIPTION:
**   Deletes a field matching the specified field name. The name is
**   conveyed using a key structure (see JsonDomGetFieldValue).
**   If the field is not found, nothing it done.
**
** NOTES:
**   The old field value, if any, is freed along with all descendants.
**
** ERROR HANDLING: It may return FALSE in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomDeleteField(jdoc, obj, nmkey) \
  ((jdoc)->mthds_jznDomDoc->jznDomDeleteField((jdoc), (obj), (nmkey)))

/*
** NAME:
**   JsonDomDeleteFieldByName
**
** PARAMETERS:
**   jznDomDoc          *jdoc  (IN) - JSON DOM
**   jznDomObject       *obj   (IN) - object within the DOM
**   oratext            *fname (IN) - Field name string
**   ub2                 fnlen (IN) - Length of field name string in bytes
**
** RETURNS:
**   (boolean)  TRUE on success, FALSE if the field was not found
**
** DESCRIPTION:
**   Deletes a field matching the specified field name.
**   If the field is not found, nothing it done.
**
** NOTES:
**   The old field value, if any, is freed along with all descendants.
**
** ERROR HANDLING: It may return FALSE in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomDeleteFieldByName(jdoc, obj, fname, fnlen) \
  ((jdoc)->mthds_jznDomDoc->jznDomDeleteFieldByName((jdoc), (obj), \
                                                    (fname), (fnlen)))

/*
** NAME:
**   JsonDomUnlinkField
**
** PARAMETERS:
**   jznDomDoc          *jdoc  (IN)  - JSON DOM
**   jznDomObject       *obj   (IN)  - object within the DOM
**   jznDomObjFieldName *nmkey (IN)  - Field key structure
**
** RETURNS:
**   (jznDomNode *)  the field value node (if any)
**
** DESCRIPTION:
**   Unlinks a field value from the DOM, returning the field value node.
**   The returned node becomes an "orphan", meaning that it is no longer
**   reachable by traversing the DOM from the root node.
**
** NOTES:
**   If the field is not found, a NULL is returned.
*/
#define JsonDomUnlinkField(jdoc, obj, nmkey) \
  ((jdoc)->mthds_jznDomDoc->jznDomUnlinkField((jdoc), (obj), (nmkey)))

/*
** NAME:
**   JsonDomUnlinkFieldByName
**
** PARAMETERS:
**   jznDomDoc          *jdoc  (IN) - JSON DOM
**   jznDomObject       *obj   (IN) - object within the DOM
**   oratext            *fname (IN) - Field name string
**   ub2                 fnlen (IN) - Length of field name string in bytes
**
** RETURNS:
**   (jznDomNode *)  the field value node (if any)
**
** DESCRIPTION:
**   Unlinks a field value from the DOM, returning the field value node.
**   The returned node becomes an "orphan", meaning that it is no longer
**   reachable by traversing the DOM from the root node.
**
** NOTES:
**   If the field is not found, a NULL is returned.
*/
#define JsonDomUnlinkFieldByName(jdoc, obj, fname, fnlen) \
  ((jdoc)->mthds_jznDomDoc->jznDomUnlinkFieldByName((jdoc), (obj), \
                                                    (fname), (fnlen)))

/*
** NAME:
**   JsonDomRenameField
**
** PARAMETERS:
**   jznDomDoc          *jdoc       (IN) - JSON DOM
**   jznDomObject       *obj        (IN) - object within the DOM
**   oratext            *oldName    (IN) - old field to be renamed
**   oratext            *oldNameLen (IN) - length of old field name
**   oratext            *newName    (IN) - new name for field
**   oratext            *newNameLen (IN) - length of new field name
**
** RETURNS:
**   jsonerr
**
** DESCRIPTION:
**   Unlinks a field value from the DOM and re-attached it with a new name.
**   If the field is not found, or if the new name is already present,
**   an error is returned.
**
** ERROR HANDLING: It may return errcode in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomRenameField(jdoc, obj, on, onl, nn, nnl) \
  ((jdoc)->mthds_jznDomDoc->jznDomRenameField((jdoc), (obj), \
                                              (on), (onl), (nn), (nnl)))

/*
** NAME:
**   JsonDomDeleteElement
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   ub4            pos      (IN)  - position within the array (0-based)
**
** RETURNS:
**   (boolean)  TRUE on success, FALSE on error
**
** DESCRIPTION:
**   Remove an element at a specified position within the array, contracting
**   the array by 1 element.
**   The position given must be within the array bounds.
**
** NOTES:
**   The removed element node is freed along with all descendants.
**
** ERROR HANDLING: It may return FALSE in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomDeleteElement(jdoc, arr, idx)\
  ((jdoc)->mthds_jznDomDoc->jznDomDeleteItem((jdoc), (arr), (idx)))

/*
** NAME:
**   JsonDomUnlinkElement
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   ub4            pos      (IN)  - position within the array (0-based)
**
** RETURNS:
**   (jznDomNode *)  the element value node, or NULL if out of bounds
**
** DESCRIPTION:
**   Remove an element at a specified position within the array, contracting
**   the array by 1 element. The removed element becomes an "orphan" and is
**   returned.
**   The position given must be within the array bounds.
**
** ERROR HANDLING: It may return NULL in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomUnlinkElement(jdoc, arr, idx)\
  ((jdoc)->mthds_jznDomDoc->jznDomUnlinkItem((jdoc), (arr), (idx)))

/*
** NAME:
**   JsonDomDeleteElementRange
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   ub4            start    (IN)  - position within the array (0-based)
**   ub4            deleteSz (IN)  - number of elements to delete
**
** RETURNS:
**   (ub4)  number of elements removed from the array.
**
** DESCRIPTION:
**   Removes a range of elements from an array, contracting it by the
**   number of elements removed. The start position given should be within
**   the array bounds, and refers to the first position to be removed
**   (i.e. a 0 implies that the first element will be removed). The
**   deleteSz may be larger than the size of the array following the
**   start position, in which case the array is truncated at the start
**   position. The number of elements actually removed will be returned
**   (this may be 0).
**
** NOTES:
**   The removed element nodes are freed along with all descendants.
*/
#define JsonDomDeleteElementRange(jdoc, arr, start, deleteSz) \
  ((jdoc)->mthds_jznDomDoc->jznDomDeleteItemBatch((jdoc), (arr), \
                                                  (start), (deleteSz)))

/*
** NAME:
**   JsonDomUnlinkElements
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   ub4            pos      (IN)  - position within the array (0-based)
**   ub4            numNodes (IN)  - number of elements to unlink
**   jznDomNode   **nodes    (OUT) - nodes removed (optional)
**
** RETURNS:
**   (ub4)  number of elements removed from the array.
**
** DESCRIPTION:
**   Removes a range of elements from an array, contracting it by the
**   number of elements removed. The start position given should be within
**   the array bounds, and refers to the first position to be removed
**   (i.e. a 0 implies that the first element will be removed). The
**   deleteSz may be larger than the size of the array following the
**   start position, in which case the array is truncated at the start
**   position. The number of elements actually removed will be returned
**   (this may be 0).
**
** NOTES:
**   Elements removed become "orphans". If the optional nodes array is
**   given, it's expected to be large enough to hold the unlinked nodes.
*/
#define JsonDomUnlinkElements(jdoc, arr, pos, numNodes, nodes) \
  ((jdoc)->mthds_jznDomDoc->jznDomUnlinkElems((jdoc), (arr), (pos), \
                                              (numNodes), (nodes)))

/*
** NAME:
**   JsonDomInsertElements
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *arr      (IN)  - array within the DOM
**   ub4            pos      (IN)  - position within the array (0-based)
**   ub4            numNodes (IN)  - number of elements to insert
**   jznDomNode   **nodes    (IN)  - nodes to be inserted
**
** RETURNS:
**   (ub4)  number of nodes inserted
**
** DESCRIPTION:
**   Inserts a set of orphan nodes as new elements in an array. The insertion
**   is done at the position specified, with 0 meaning that the inserted
**   nodes becomes the first N elements in the array, and all other elements
**   are shifted, increasing the size of the array by N element. The
**   array size bound may be specified as the insert position as a means
**   of appending nodes to the end of the array. The elements are inserted
**   in order, effectively advancing the insert position by 1 each time.
**
** ERROR HANDLING: It may return a number less than the requested node count.
** with errrcode obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomInsertElements(jdoc, arr, pos, numNodes, nodes) \
  ((jdoc)->mthds_jznDomDoc->jznDomInsertItems((jdoc), (arr), (pos), \
                                              (numNodes), (nodes)))

/*------------------------ Constructor interfaces ---------------------------*/

/*
** NAME:
**   JsonDomCreateObject
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**
** RETURNS:
**   (jznDomObject *)  new orphaned DOM object node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM object node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
*/
#define JsonDomCreateObject(jdoc) \
  ((jdoc)->mthds_jznDomDoc->jznDomNewObject((jdoc), 0))

/*
** NAME:
**   JsonDomCreateObjectWithSize
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   ub4            sz       (IN)  - initial number of field elements in the obj
**                           if 0, default size is used. Passing non zero
**                           size helps to optimize internal memory allocation
**                           and avoid re-sizing if we know the number of
**                           field elements it needs in advance.
**
**
** RETURNS:
**   (jznDomObject *)  new orphaned DOM object node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM object node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
*/
#define JsonDomCreateObjectWithSize(jdoc, sz) \
  ((jdoc)->mthds_jznDomDoc->jznDomNewObject((jdoc), (sz)))

/*
** NAME:
**   JsonDomCreateArray
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**
** RETURNS:
**   (jznDomArray *)  new orphaned DOM array node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM array node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
*/
#define JsonDomCreateArray(jdoc) \
  ((jdoc)->mthds_jznDomDoc->jznDomNewArray((jdoc), 0))

/*
** NAME:
**   JsonDomCreateArrayWithSize
**
** PARAMETERS:
**   jznDomDoc    *jdoc      (IN)  - JSON DOM
**   ub4           sz        (IN)  - initial number of chd elements in the array
**                           if 0, default size is used. Passing non zero
**                           size helps to optimize internal memory allocation
**                           and avoid re-sizing if we know the number of
**                           field elements it needs in advance.
**
** RETURNS:
**   (jznDomArray *)  new orphaned DOM array node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM array node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
*/
#define JsonDomCreateArrayWithSize(jdoc, sz) \
  ((jdoc)->mthds_jznDomDoc->jznDomNewArray((jdoc), (sz)))

/*
** NAME:
**   JsonDomCreateScalar
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznScalarVal  *val      (IN)  - Scalar value structure
**
** RETURNS:
**   (jznDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value conveyed by the discriminated union val.
**   For variable-length values (such as strings), the bytes representing
**   the value are copied into the DOM's memory space.
*/
#define JsonDomCreateScalar(jdoc, val) \
  ((jdoc)->mthds_jznDomDoc->jznDomNewScalar((jdoc), (val)))

/*
** NAME:
**   JsonDomCopyScalar
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomScalar  *snode    (IN)  - Existing scalar node
**
** RETURNS:
**   (jznDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node as a duplicate of an existing scalar
**   node. The new node is an "orphan" not attached to the DOM tree that
**   descends from the root node. If possible, the scalar data value is
**   shared in a read-only fashion between the nodes, allowing logical
**   copies to be made of nodes with long out-of-line data values such
**   as strings or binaries.
*/
#define JsonDomCopyScalar(jdoc, snode) \
  ((jdoc)->mthds_jznDomDoc->jznDomDupScalar((jdoc), (snode)))

/*
** NAME:
**   JsonDomSetScalar
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomScalar  *snode    (IN)  - Existing scalar node
**   jznScalarVal  *val      (IN)  - scalar value to set
**
** RETURNS:
**   (jznDomScalar *) updated orphan DOM scalar node.
**   NULL if the feature is not supported.
**
** DESCRIPTION:
**   Updates an existing DOM scalar node to set the value to a new
**   scalar value. This is done without deep-copying the value, so
**   the memory must remain stable while the node is in use.
**   Pass a NULL for <val> to clear the scalar node to its original state.
**   if <snode> is NULL, a suitable "temp" scalar is created and then
**   the value is pushed to it.
**
** NOTES:
**   This is a dangerous API that must be used with caution.
**   It's designed for internal use to carry transient values to
**   APIs that require a node.
*/
#define JsonDomSetScalar(jdoc, snode, val) \
  ((jdoc)->mthds_jznDomDoc->jznDomSetScalar((jdoc), (snode), (val)))

/*
** NAME:
**   JsonDomCreateScalarVal
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   jznvaltyp      vtyp     (IN)  - Scalar value type
**   va_list        values   (IN)  - Scalar value
**                                   (paired with length if applicable)
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value conveyed by the varargs list.
**   For variable-length values (such as strings), the bytes representing
**   the value are copied into the DOM's memory space.
*/
#define JsonDomCreateScalarVal(jdoc, vtyp, ...) \
  ((jdoc)->mthds_jznDomDoc->jznDomNewScalarVal((jdoc), (vtyp), __VA_ARGS__))

/*
** NAME:
**   JsonDomCreateString
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   oratext       *sval     (IN)  - String pointer
**   ub4            slen     (IN)  - String size in bytes
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateString(jdoc, sval, slen) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_STRING, (sval), (slen)))

/*
** NAME:
**   JsonDomCreateBinary
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub1           *bval     (IN)  - Binary buffer pointer
**   ub4            blen     (IN)  - Buffer size in bytes
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateBinary(jdoc, bval, blen) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_BINARY, (bval), (blen)))

/*
** NAME:
**   JsonDomCreateVector
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub1           *bval     (IN)  - Vector Binary buffer pointer
**   ub4            blen     (IN)  - Vector Buffer size in bytes
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateVector(jdoc, bval, blen) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_VECTOR, (bval), (blen)))

/*
** NAME:
**   JsonDomCreateInteger
**
** PARAMETERS:
**   JsonDomDoc     *jdoc     (IN)  - JSON DOM
**   sb4             ival     (IN)  - Integer value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateInteger(jdoc, ival) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_SINT, (ival)))

/*
** NAME:
**   JsonDomCreateUnsignedInteger
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub4            ival     (IN)  - Integer value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateUnsignedInteger(jdoc, ival) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_UINT, (ival)))

/*
** NAME:
**   JsonDomCreateLong
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   sb8            ival     (IN)  - Long value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateLong(jdoc, lnval) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_SLONG, (lnval)))

/*
** NAME:
**   JsonDomCreateUnsignedLong
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub8            ival     (IN)  - Long value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateUnsignedLong(jdoc, lnval) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_ULONG, (lnval)))

/*
** NAME:
**   JsonDomCreateFloat
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   float          fval     (IN)  - Float value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateFloat(jdoc, fval) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_FLOAT, (fval)))

/*
** NAME:
**   JsonDomCreateDouble
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   double         dval     (IN)  - Double value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateDouble(jdoc, dval) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DOUBLE, (dval)))

/*
** NAME:
**   JsonDomCreateShort
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   short          ival     (IN)  - 16-bit signed integer value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateShort(jdoc, ival) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_SHORT, (ival)))

/*
** NAME:
**   JsonDomCreateUShort
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   short          uval     (IN)  - 16-bit unsigned integer value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
#define JsonDomCreateUshort(jdoc, ival) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_USHORT, (ival)))

// ### To-do: CreateFloat16, CreateBFloat16 - but how?

/*
** NAME:
**   JsonDomCreateTrue
**
** PARAMETERS:
**   JsonDomDoc     *jdoc     (IN)  - JSON DOM
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the TRUE.
*/
#define JsonDomCreateTrue(jdoc) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_TRUE, NULL))

/*
** NAME:
**   JsonDomCreateFalse
**
** PARAMETERS:
**   JsonDomDoc     *jdoc     (IN)  - JSON DOM
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the FALSE.
*/
#define JsonDomCreateFalse(jdoc) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_FALSE, NULL))

/*
** NAME:
**   JsonDomCreateBoolean
**
** PARAMETERS:
**   JsonDomDoc     *jdoc     (IN)  - JSON DOM
**   boolean         bval     (IN)  - TRUE or FALSE
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the boolean value passed.
*/
#define JsonDomCreateBoolean(jdoc, bval) \
  ((bval) ? JsonDomCreateTrue(jdoc) : JsonDomCreateFalse(jdoc))

/*
** NAME:
**   JsonDomCreateNull
**
** PARAMETERS:
**   JsonDomDoc     *jdoc     (IN)  - JSON DOM
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to a JSON null.
*/
#define JsonDomCreateNull(jdoc) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_NULL, NULL))

/*
** NAME:
**   JsonDomCreateNumber
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub1           *nval     (IN)  - Pointer to byte array
**   ub4            nlen     (IN)  - Buffer size in bytes
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateNumber(jdoc, nval, nlen) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_ORA_NUMBER, (nval), (nlen)))

/*
** NAME:
**   JsonDomCreateDate
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub1           *dval     (IN)  - Pointer to byte array
**   ub4            dlen     (IN)  - Buffer size in bytes
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateDate(jdoc, dval, dlen) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_ORA_DATE, (dval), (dlen)))

/*
** NAME:
**   JsonDomCreateTimestamp
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub1           *tsval    (IN)  - Pointer to byte array
**   ub4            tslen    (IN)  - Buffer size in bytes
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateTimestamp(jdoc, tsval, tslen) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_ORA_TIMESTAMP, (tsval), (tslen)))

/*
** NAME:
**   JsonDomCreateId
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   ub1           *bval     (IN)  - Binary buffer pointer
**   ub4            blen     (IN)  - Buffer size in bytes
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateId(jdoc, bval, blen) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_ID, (bval), (blen)))

/*
** NAME:
**   JsonDomNumberFromLong
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   sb8            ival     (IN)  - Long value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
JsonDomScalar *JsonDomNumberFromLong(JsonDomDoc *jdoc, sb8 ival);

/*
** NAME:
**   JsonDomNumberFromDouble
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   double         dval     (IN)  - Double value
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
*/
JsonDomScalar *JsonDomNumberFromDouble(JsonDomDoc *jdoc, double dval);

/*
** NAME:
**   JsonDomFloat16FromFloat
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   float          fval     (IN)  - 32-bit float
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node, IEEE 754 16-bit float
**   NULL on an out of memory condition or if the input cannot be converted
**   due to the smaller scale of the 16-bit float.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
JsonDomScalar *JsonDomFloat16FromFloat(JsonDomDoc *jdoc, float fval);

/*
** NAME:
**   JsonDomBFloat16FromFloat
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   float          fval     (IN)  - 32-bit float
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node, Google 16-bit float
**   NULL on an out of memory condition or if the input cannot be converted.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
JsonDomScalar *JsonDomBFloat16FromFloat(JsonDomDoc *jdoc, float fval);

/*
** NAME:
**   JsonDomCreateStream
**
** PARAMETERS:
**   JsonDomDoc     *jdoc     (IN)  - JSON DOM
**   orastream      *strm     (IN)  - Read input stream
**   is_bin          is_bin   (IN)  - TRUE
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition, or on a stream reading error;
**   call JsonDomGetError() if this occurs.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value read from the input read stream.
**   The stream bytes are read and copied into the DOM's memory space.
*/
#define JsonDomCreateStream(jdoc, strm, is_bin) \
  ((is_bin) ? (JsonDomCreateScalarVal((jdoc), JZNVAL_BIN_STREAM, (strm))) : \
              (JsonDomCreateScalarVal((jdoc), JZNVAL_CHR_STREAM, (strm))))

/*
** NAME:
**   JsonDomCreateOCIString
**
** PARAMETERS:
**   JsonDomDoc    *jdoc    (IN)   - JSON DOM
**   OCIString     *ostr    (IN)   - OCIString instance
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateOCIString(jdoc, ostr) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_OCI_STRING, (ostr)))

/*
** NAME:
**   JsonDomCreateOCIRaw
**
** PARAMETERS:
**   JsonDomDoc    *jdoc    (IN)   - JSON DOM
**   OCIRaw        *oraw    (IN)   - OCIRaw instance
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateOCIRaw(jdoc, oraw) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_OCI_RAW, (oraw)))

/*
** NAME:
**   JsonDomCreateOCINumber
**
** PARAMETERS:
**   JsonDomDoc    *jdoc    (IN)  - JSON DOM
**   OCINumber     *onum    (IN)   - OCINumber instance
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateOCINumber(jdoc, onum) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_OCI_NUMBER, (onum)))

/*
** NAME:
**   JsonDomCreateOCIDate
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   OCIDate       *odate    (IN)  - OCIDate instance
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateOCIDate(jdoc, odate) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_OCI_DATE, (odate)))

/*
** NAME:
**   JsonDomCreateOCIDateTime
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   OCIDateTime   *otime    (IN)  - OCIDateTime instance
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateOCIDateTime(jdoc, otime) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_OCI_DATETIME, (otime)))

/*
** NAME:
**   JsonDomCreateOCIInteval
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   OCIInterval   *oint     (IN)  - OCIInterval instance
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is set to the value passed in as parameter.
**   The bytes representing the value are copied into the DOM's memory space.
*/
#define JsonDomCreateOCIInterval(jdoc, oint) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_OCI_INTERVAL, (oint)))

/*
** NAME:
**   JsonDomCreateDecimal
**   JsonDomCreateDecimalString
**   JsonDomCreateDecimalInt
**   JsonDomCreateDecimalLong
**   JsonDomCreateDecimalFloat
**   JsonDomCreateDecimalDouble
**   JsonDomCreateDecimalUnsigned
**   JsonDomCreateDecimalUL
**
** PARAMETERS:
**   JsonDomDoc    *jdoc    (IN)  - JSON DOM
**   <various>      x       (IN)  - Numeric input
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is an in-line IEEE Decimal 128. The input may be any of
**   the following: signed int, unsigned int, signed long, unsigned long,
**   float, double, or OCI Number.
*/
#define JsonDomCreateDecimal(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_OCI_NUMBER, (x)))
#define JsonDomCreateDecimalString(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_STRING, (x)))
#define JsonDomCreateDecimalInt(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_SINT, (x)))
#define JsonDomCreateDecimalLong(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_SLONG, (x)))
#define JsonDomCreateDecimalUnsigned(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_UINT, (x)))
#define JsonDomCreateDecimalUL(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_ULONG, (x)))
#define JsonDomCreateDecimalFloat(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_FLOAT, (x)))
#define JsonDomCreateDecimalDouble(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_DECIMAL, JZNVAL_DOUBLE, (x)))

/*
** NAME:
**   JsonDomCreateDateTime
**   JsonDomCreateDateTimeMillis
**   JsonDomCreateDateTimeString
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   <various>      x        (IN)  - OCIDateTime instance, or
**                                   milliseconds since 1970-01-01, or
**                                   ISO string (null-terminated)
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is an in-line date/time structure.
*/
#define JsonDomCreateDateTime(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_TSTAMP, JZNVAL_OCI_DATETIME, (x)))
#define JsonDomCreateDateTimeMillis(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_TSTAMP, JZNVAL_SLONG, (x)))
#define JsonDomCreateDateTimeString(jdoc, x) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_TSTAMP, JZNVAL_STRING, (x)))

/*
** NAME:
**   JsonDomCreateUUID
**   JsonDomCreateOID
**
** PARAMETERS:
**   JsonDomDoc    *jdoc     (IN)  - JSON DOM
**   oratext       *idstr    (IN)  - Hexadecimal string
**
** RETURNS:
**   (JsonDomScalar *)  new orphaned DOM scalar node
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   Creates a new DOM scalar node. The new node is an "orphan", meaning
**   it is not attached to the DOM tree that descends from the root node.
**   The scalar is an in-line UUID or OID value.
*/
#define JsonDomCreateUUID(jdoc, idstr) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_UUID, (idstr)))
#define JsonDomCreateOID(jdoc, idstr) \
  (JsonDomCreateScalarVal((jdoc), JZNVAL_OID, (idstr)))

/*
** Create a JZNVAL_SCALAR_ARRAY instance from an input JSON array.
** The input array must contain scalar nodes compatible with the
** requested array element type <vtype>.
*/
JsonDomScalar *jznDomScalarArray(JsonDomDoc *srcDoc, JsonDomArray *srcArr,
                                 JsonDomDoc *outDoc, jznvaltype vtype);

/*------------------------- Utility interfaces ------------------------------*/

/*
** NAME:
**   JsonDomClear
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**
** RETURNS:
**   (void)
**
** DESCRIPTION:
**   Clears all state associated with a DOM, returning it to the "new"
**   state and eliminating the root node (if any) and all descendants,
**   as well as all orphan nodes.
**
** NOTES:
**   This allows a DOM to be a reusable container into which a series
**   of serialized JSON documents may be loaded, processed, and unloaded.
*/
#define JsonDomClear(jdoc) \
  ((jdoc)->mthds_jznDomDoc->jznDomDocReset((jdoc)))

/*
** NAME:
**   JsonDomFree
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**
** RETURNS:
**   (void)
**
** DESCRIPTION:
**   Release all memory associated with a DOM, destroying it.
*/
#define JsonDomFree(jdoc) \
  ((jdoc)->mthds_jznDomDoc->jznDomDocFree((jdoc)))

/*
** NAME:
**   JsonDomFreeNode
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomNode    *nd       (IN)  - Node to free
**
** RETURNS:
**   boolean - TRUE if freed, FALSE if not an orphan
**
** DESCRIPTION:
**   Free an orphan node and all descendants.
*/
#define JsonDomFreeNode(jdoc, nd) \
  ((jdoc)->mthds_jznDomDoc->jznDomFreeNode((jdoc),(nd)))

/*
** NAME:
**   JsonDomToEventSource
**
** PARAMETERS:
**   jznDomDoc     *jdoc      (IN)  - DOM to use as event source
**
** RETURNS:
**   (jznEventSource *)  An event source backed by the DOM
**
** DESCRIPTION:
**   Returns an event source that can be used to consume the DOM as a series
**   of events, e.g. in order to write it to an encoder in another format.
**   The events begin with the root node.
**
** NOTES:
**   The event source is backed by the DOM and is a stateful object that
**   simply walks the DOM tree. The DOM should not undergo mutation while
**   events are being produced.
**   ### Internally we should check this using the modification counter.
**   The event source should not be destroyed conventionally, because it's
**   a captive sub-object of the DOM itself.
*/
#define JsonDomToEventSource(jdoc)                        \
  ((jdoc)->mthds_jznDomDoc->jznDomGetOutputEventSrc((jdoc)))

/*
** NAME:
**   JsonDomEquals
**
** PARAMETERS:
**   jznDomDoc     *jdoc1     (IN)  - First DOM
**   jznDomNode    *nd1       (IN)  - Node within first DOM
**   jznDomDoc     *jdoc2     (IN)  - Second DOM
**   jznDomNode    *nd2       (IN)  - Node within second DOM
**
** RETURNS:
**   (boolean)  TRUE if the nodes are identical
**
** DESCRIPTION:
**   This performs a "deep equals" on two nodes which may be from the same
**   DOM or from two different DOMs. Objects are compared by matching them
**   field-by-field using the names, and arrays are compared by matching
**   them position-by-position. Scalars are compared using domain-specific
**   comparators.
**
** NOTES:
**   Scalars are converted to canonical representations for comparison.
**   Thus, 1E2 and 100 should both compare as equal numerically.
**
** ### The comparison method from the first document is used.
*/
#define JsonDomEquals(jdoc1, nd1, jdoc2, nd2) \
  ((jdoc1)->mthds_jznDomDoc->jznDomEquals((jdoc1), (nd1), (jdoc2), (nd2)))

/*
** NAME:
**   JsonDomCopy
**
** PARAMETERS:
**   jznDomDoc     *srcdoc    (IN)  - Source DOM
**   jznDomNode    *srcnode   (IN)  - Source node within source DOM
**   jznDomDoc     *destdoc   (IN)  - Target DOM
**
** RETURNS:
**   (jznDomNode *)  Copy of the node in the target DOM
**   NULL on an out of memory condition.
**
** DESCRIPTION:
**   This performs a "deep copy" of a source node to a new node, possibly
**   in another DOM. The root of the copy is returned as an "orphan" node
**   in the target DOM. The target DOM may be the same as the source DOM.
**
** NOTES:
**   The copy method from the target document is used.
*/
#define JsonDomCopy(srcdoc, srcnode, destdoc) \
  ((destdoc)->mthds_jznDomDoc->jznDomCopy((srcdoc), (srcnode), (destdoc)))

/*
** NAME:
**   JsonDomCheckFieldKeys
**
** PARAMETERS:
**   jznDomDoc          *jdoc      (IN)  - Target DOM
**   jznDomObjFieldName *fnms      (IN)  - Array of field name structures
**   ub2                 fnmsn     (IN)  - number of field names in the array
**
** RETURNS:
**   (jsonerr)  error code, JZNERR_OK (0) for no error
**
** DESCRIPTION:
**   Validates a set of field IDs against a target DOM. The fields are
**   conveyed as an array of key structures identical to that used by
**   JsonDomGetFieldValue. The fields are checked against the target DOM
**   to see if the fast-access IDs are identical and therefore safe to
**   use against the target DOM. If not, they're set to 0.
**
** NOTES:
**   This is needed because each DOM instance may, for binary formats,
**   assign different numeric key IDs to field names, based on the order
**   they are encountered. (And, in some cases, an instance may never have
**   encountered a particular field name.) IDs provide faster access than
**   using hash codes, but IDs aren't in general portable across instances,
**   whereas hash codes are always the same.
*/
#define JsonDomCheckFieldKeys(jdoc, fnms, fnmsn) \
  ((jdoc)->mthds_jznDomDoc->jznDomValidFid(jdoc, fnms, fnmsn))

/*
** NAME:
**   JsonDomStoreFieldName
**
** PARAMETERS:
**   jznDomDoc          *jdoc      (IN)  - Target DOM
**   oratext            *fname     (IN)  - Field name string
**   ub4                 fnlen     (IN)  - Length of field name string
**   jznDomObjFieldName *nmkey     (OUT) - Field name structure
**
** RETURNS:
**   (jsonerr)  error code, JZNERR_OK (0) for no error
**
** DESCRIPTION:
**   Ensures that a field name is stored durably in the DOM, and populates
**   a field name key structure with the result.
**
** NOTES:
**   If the field name is already stored durably, the existing value is
**   returned and no change is made to the DOM memory.
*/
#define JsonDomStoreFieldName(jdoc, fname, fnlen, nmkey) \
  ((jdoc)->mthds_jznDomDoc->jznDomStoreField((jdoc),(fname),(fnlen),(nmkey)))

/*
** Deep copy a tree of nodes from a source DOM into a target DOM.
** The two DOMs may be the same. The head of the new tree is returned.
*/
JsonDomNode *jznDomCopy(JsonDomDoc *srcdoc, JsonDomNode *srcroot,
                        JsonDomDoc *destdoc);

/*
** Compute a "deep equals" between two nodes from two possibly different DOMs.
*/
boolean jznDomEquals(JsonDomDoc *doc1, JsonDomNode *root1,
                     JsonDomDoc *doc2, JsonDomNode *root2);

/* Given two JSON scalar values, tell whether they are the same */
boolean jznScalarEquals(jznScalarVal *val1, jznScalarVal *val2);

/*
** Compare two nodes returning
**    > 0 if node1 > node2
**    < 0 if node1 < node2
**      0 if node1 == node2
*/
sword jznDomCompare(JsonDomDoc *doc1, JsonDomNode *node1,
                    JsonDomDoc *doc2, JsonDomNode *node2);

/*
** Perform a deep comparison of two nodes from two documents.
** Returns the status code if anything goes wrong, otherwise returns
** JZNERR_OK and the result of the comparison is found in <result>.
** Returns > 0, < 0, or 0 for node1 > node2, node1 < node2, or equality.
*/
#define JZNDOM_COMPARE_FLAG_EQUALS     0x0001
#define JZNDOM_COMPARE_FLAG_ALLOC      0x0002
#define JZNDOM_COMPARE_FLAG_BRACKET    0x0004
#define JZNDOM_COMPARE_FLAG_TYPECHECK  0x0008
#define JZNDOM_COMPARE_FLAG_TEMP       0x0010

sword jznDomCompareEx(JsonDomDoc *doc1, JsonDomNode *node1,
                      JsonDomDoc *doc2, JsonDomNode *node2,
                      sword *result, uword flags);

/* typeBracket = TRUE means node1 and node2 is of the same kind of node
 * (object, array, scalar of compatabile type). Otherwise, it returns
 * JZNERR_INCOMPATIBLE_COMPARE.
 */
/* Bug34615330: jznDomCompareWithTypeBracket() */
/* when typeBracket is TRUE, the node1 and node2 must of the same
 * node type
 * (object node vs object node, array  node vs array node,
 * scalar node vs scalar node of compatible scalar value type
 * at top level.
 * It internally passes down JZNANY_FLAG_MATCH_DATA_TYPE to achieve this.
 *
 * It returns JZNERR_INCOMPATIBLE_COMPARE if node is NOT compatible.
 */
sword jznDomCompareWithTypeBracket(JsonDomDoc *doc1, JsonDomNode *node1,
                                   JsonDomDoc *doc2, JsonDomNode *node2,
                                   sword *result, uword flags,
                                   boolean typeBracket);

/*
** Sort all fields in a DOM object node (using heapsort in-place)
*/
void jznDomSortFields(jznDomNameValuePair *nvps, ub4 numfields);

/*
** Get the scalar value type for a node.
** If the node is non-scalar, returns a generic JZNVAL_DOC_NODE.
*/
jznvaltype JsonDomGetScalarType(JsonDomDoc *doc, JsonDomNode *node);

/*--------------------------- Serializer interfaces -------------------------*/

/*
** NAME:
**   JsonSerializeDom
**
** PARAMETERS:
**   jznDomDoc     *jdoc    (IN)  - DOM to be serialized
**   list                   (IN)  - NULL-terminated list of variable arguments
**
**   Supported varargs parameters:
**   =======================================================================
**   Property Name       Expected Type     Valid values      Notes
**   =======================================================================
**   JZN_FILE            oratext *
**   JZN_BUFFER          void *
**   JZN_BUFFERLEN       ub4 *                               IN/OUT
**   JZN_STREAM          orastream *
**   JZN_FORMAT          oratext *         "JSON", "OSON"
**   JZN_PRINT_FLAGS     ub4                                 For "JSON" only
**   JZN_DOM             JsonDomDoc *                        Must for "OSON"
**
** RETURNS:
**   (jsonerr)
**
** DESCRIPTION:
**   Serializes a DOM and writes to a sink specified in the varargs list. The
**   output format can either be textual JSON (JZN_FORMAT = "JSON") or binary
**   OSON (JZN_FORMAT = "OSON").
**
** NOTES:
**   If output format requested is OSON, source DOM should be JZN_INMEM_DOM
**   type.
**
*/
#define JsonSerializeDom(jdoc, ...) \
  ((jdoc)->mthds_jznDomDoc->jznDomSerialize((jdoc), __VA_ARGS__))

/*
** NAME:
**   JsonDomPrintDocument
**
** PARAMETERS:
**   jznDomDoc     *jdoc      (IN)  - DOM to be printed
**   jznPrintCtx   *printCtx  (IN)  - Print context to write the DOM to
**
** RETURNS:
**   (jsonerr) - Error code
**
** DESCRIPTION:
**   Serializes a DOM to a print context. The print context is a type of
**   event writer or "sink". The DOM is walked from the root, and events
**   delivered to the print context, which serializes the DOM as valid JSON.
**
** NOTES:
**   The caller is expected to have set up the print context before calling
**   this interface. The print context may be backed by a stream or a
**   fixed sized buffer, or by an extensible buffer internal to the
**   print context.
** 1. Before this function is called for the first time, call
**    jznuPrintCreate() to initialize the printCtx.
** 2. Once a jznuPrintCtx is created, it can be reused to serialize
**    multiple documents by calling this function multiple times.
** 3. When done with serializing all documents, call
**    jznuPrintDestroy() to destroy the printCtx.
*/
#define JsonDomPrintDocument(jdoc, printCtx) \
  jznDomPrintDocument((jdoc), (printCtx))

/*
** NAME:
**   JsonDomPrintNode
**
** PARAMETERS:
**   jznDomDoc      *jdoc     (IN)  - DOM containing nodes to be printed
**   jznDomNode     *node     (IN)  - node within the DOM to serialize
**   jznEventWriter *writer   (IN)  - Event writer to consume the events
**
** RETURNS:
**   (jsonerr) - Error code
**
** DESCRIPTION:
**   Serializes a DOM node tree to an event writer. The event writer is
**   commonly a print context, but may also be another event consumer
**   such as a binary encoder
**
** NOTES:
**   This interface is a more general version of JsonDomPrintDocument.
**   JsonDomPrintDocument is equivalent to calling this interface with
**   jznuPrintToWriter() around the print context and JsonDomGetRoot()
**   as the starting node.
*/
/* instead of calling generic  jznDomPrintNode((jdoc), (node), (writer)),
 * we call each DOM implmentation method which is more efficient due to
 * direct internal func call
 */
#define JsonDomPrintNode(jdoc, node, writer) \
  ((jdoc)->mthds_jznDomDoc->jznDomPrintNode((jdoc), (node), (writer)))

/*
** NAME:
**    JsonDomSave
**
**  PARAMETERS:
**    JsonDomDoc      *jdoc     (IN)  - DOM containing nodes to be printed
**    jsonerr         *jerr     (OUT)  - Error code
**
**  RETURNS:
**    (ubig_ora) - Number of bytes return to the output sink
**
**  DESCRIPTION:
**    Serializes a DOM node tree to a sink. The sink can be specified using the
**    varargs.
**
**  NOTES:
**    This interface is a more general version of JsonDomPrintDocument.
**    Supported varargs parameters:
**    ===================================
**    Property Name       Expected Type
**    ===================================
**    JZN_FILE            oratext *
**    JZN_BUFFER          void *
**    JZN_BUFFERLEN       ub4
**    JZN_STREAM          orastream *
**    JZN_PRETTY          boolean
**    JZN_ASCII           boolean
**    JZN_EXTENSIBLE_BUF  boolean
*/
ubig_ora JsonSaveDom(JsonDomDoc  *jdoc, jsonerr  *jerr, ...);

ubig_ora JsonSaveDomVA(JsonDomDoc  *jdoc, jsonerr  *jerr, va_list va);

/*
** NAME:
**  jznDomScalarToString
**
** DESCRIPTION:
**   Convert a jznScalarVal to its textual representation
**
** INPUTS:
**   xmlctx       *xctx   : XML context (NULL OK)
**   jznScalarVal *val    : Scalar value to convert
**   char         *outbuf : Buffer to hold the result
**   ub4          *outlen : IN/OUT length of the buffer 'outbuf'
**
** OUTPUTS:
**   sword : indicates the status of the conversion.
**           JZNERR_OK means conversion is successful.
**           All callers should check the return status before proceeding.
*/
sword jznDomScalarToString(xmlctx *xctx, jznScalarVal *val,
                           char *outbuf, ub4 *outlen);

/*
** Print an entire DOM to a print context.
*/
jsonerr jznDomPrintDocument(JsonDomDoc *doc, jznuPrintCtx *pctx);

#define JZNDOM_PRINT_SORT_NONE       0x0000 /* No sorting */
#define JZNDOM_PRINT_SORT_KEYNAME    0x0001 /* Sort objects by key name */
#define JZNDOM_PRINT_SORT_OPTIMIZE   0x0002 /* Optimize field ordering */
#define JZNDOM_PRINT_SORT_METADATA   0x0004 /* put _id/_etag first */
#define JZNDOM_PRINT_HDL_RAW_ASOSON  0x0008 /* detect oson embedded in raw*/

/*
** Print a tree of nodes from a given starting point within the DOM.
** Optionally can produce nodes in sorted order.
*/
jsonerr jznDomPrintSortNode(JsonDomDoc *doc, JsonDomNode *node,
                            jznEventWriter *writer, ub4 flags);

/*
** Print a tree of nodes from a given starting point within the DOM.
*/
jsonerr jznDomPrintNode(JsonDomDoc *doc, JsonDomNode *node,
                        jznEventWriter *writer);

/*
** NAME:
**   jznuGetScalarLength
**
** PARAMETERS:
**   val : scalar value
**
** RETURNS:
**   length of the given scalar value
**
** DESCRIPTION:
**   Returns the length of a scalar value
*/
ub4 jznuGetScalarLength(jznScalarVal *val);

/*
** Return the number of entries in the DOM hash table.
** ### Move this to jzn0.h ?
*/
ub4 jznDomHashGetSize(jznDomHashTable *ht);


/*---------------------------- OSON Utility interfaces ----------------------*/

/*
** NAME:
**   jznoctLoadInMemDomFromInputOSON
**   jznoctLoadInMemDomFromInputOSON_WF
**
** PARAMETERS:
**   JsonDomDoc        *srcdoc     (IN)  - OSON DOM container
**   JsonDomDoc        *srcroot    (IN)  - Root node of the OSON DOM
**   JsonDomDoc        *destdoc    (IN)  - In-memory DOM container
**   ub4                flag       (IN)  - Flags for loading
**
** RETURNS:
**   (JsonDomNode *) - Root node after loading
**
** DESCRIPTION:
**   Loads in-memory JSON DOM from OSON DOM backed by OSON buffer. If source
**   OSON bytes buffer in srcdoc is guaranteed to be stable during the life
**   time of the in-memory JSON DOM loaded from the OSON bytes buffer, then
**   flags can be used for optimizing in-memory DOM loading process.
**   Calling jznoctLoadInMemDomFromInputOSON() is semantically equivalent to
**   calling jznoctLoadInMemDomFromInputOSON_WF() with JZNOCT_NO_CPY_LEAF_VAL
**   flag.
**
** NOTES:
**   ERROR HANDLING: It may return NULL in case of error with errrcode
**   obtaineable via JsonDomGetError(srcdoc).
*/
JsonDomNode *jznoctLoadInMemDomFromInputOSON_WF(JsonDomDoc *srcdoc,
                                             JsonDomNode *srcroot,
                                             JsonDomDoc *destdoc, ub4 flag);

JsonDomNode *jznoctLoadInMemDomFromInputOSON(JsonDomDoc *srcdoc,
                                             JsonDomNode *srcroot,
                                             JsonDomDoc *destdoc);

/*
** Avoids deep copy of the leaf value as the leaf value resides in the OSON
** bytes buffer is stable. However, in case the leaf value needs to be
** modified, then a deep copy of leaf content will be made by then by using
** JZN0DOM_SC_VAL_IN_OSON_BUF flag.
*/
#define JZNOCT_NO_CPY_LEAF_VAL           0x1

/*
** If there is no modification of field names (adding field names) for the
** in-memory JSON DOM loaded from OSON buffer, JZNOCT_NO_CPY_FIELDNAME_BUF
** can be used so that the field name buffer point to OSON field name
** dictionary buffer withotu deep copy.
*/
#define JZNOCT_NO_CPY_FIELDNAME_BUF      0x2

/*
** OSON to in-memory JSON DOM copy with field name dictionary partially
** set up because input oosn dom is not copied from root so that destination
** dom does not need all field name dictionary from the original oson dom.
** ERROR HANDLING: It may return NULL in case of error with errrcode
** obtaineable via JsonDomGetError(srcdoc)
*/
#define JZNOCT_FIELD_NM_PARTIAL          0x4

/*
** NAME:
**   jznoctSerInMemDomToOSON
**   jznoctSerInMemDomToOSON_WF
**
** PARAMETERS:
**   JsonDomDoc        *oct      (IN)  - OSON DOM container
**   JsonDomDoc        *doc      (IN)  - Root node of the OSON DOM
**   JsonDomNode       *root     (IN)  - In-memory DOM root node
**   jznosonfile       *ofile    (IN)  - OSON writer context
**   ub4                flag     (IN)  - Flags for loading
**
** RETURNS:
**   (jsonerr) - Error code
**
** DESCRIPTION:
**   Generates OSON bytes into ofile by serializing an in-memory JSON DOM root.
**
** NOTES:
*/
jsonerr jznoctSerInMemDomToOSON_WF(JsonDomDoc *oct, JsonDomDoc *doc,
                                   JsonDomNode *root, jznosonFile *ofile,
                                   ub4 flag);

jsonerr jznoctSerInMemDomToOSON(JsonDomDoc *oct, JsonDomDoc *doc,
                                JsonDomNode *root, jznosonFile *ofile);
/*
** Free the root after serialization
*/
#define JZNOCT_SERINMEM_GC_FILED_NAMES   0x1

/*------------------------- Error handling interfaces -----------------------*/

/*
** NAME:
**   JsonDomGetError
**
** PARAMETERS:
**   jznDomDoc      *jdoc     (IN)  - DOM to check error code for
**
** RETURNS:
**   (jsonerr)  error code, JZNERR_OK (0) for no error
**
** DESCRIPTION:
**   Returns an error code for a DOM operation that failed (for example,
**   an attempt to read an array out of bounds).
*/
#define JsonDomGetError(jdoc) \
  ((jdoc)->errcode_jznDomDoc)

/*
** NAME:
**   JsonDomGetErrorMessage
**
** PARAMETERS:
**   jznDomDoc      *jdoc     (IN)  - DOM to get message for
**   jsonerr        *err      (OUT) - Error code or 0 for none
**   boolean         clear    (IN)  - TRUE to clear error after reading
**
** RETURNS:
**   (oratext *)  error message string, NULL on memory error.
**
** DESCRIPTION:
**   Returns a rendered error message for the current DOM error code.
**   Returns the empty string if there is no current error.
**   Returns the current error code in the optional <err> argument.
**   If <clear> is TRUE, the document's internal error is cleared.
*/
#define JsonDomGetErrorMessage(jdoc, err, clear) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetError((jdoc),(err),(clear)))

/*
** NAME:
**   JsonDomClearError
**
** PARAMETERS:
**   jznDomDoc      *jdoc     (IN)  - DOM to check error code for
**
** RETURNS:
**   (void)
**
** DESCRIPTION:
**   Clear the error code for the DOM, e.g. to clear an array out of bounds
**   error after reading it.
*/
#define JsonDomClearError(jdoc) \
  ((jdoc)->errcode_jznDomDoc = JZNERR_OK)

/*
** Get a rendered error message from the DOM. Returns an empty string
** if there is no error.
*/
oratext *jznDomGetErrorMessage(JsonDomDoc *doc, jsonerr *err, boolean clear);

/*
** NAME:
**   JsonDomGetModifcations
**
** PARAMETERS:
**   jznDomDoc      *jdoc     (IN)  - DOM to get modification count for
**
** RETURNS:
**   (ub4)  running count of mutation operations against the DOM
**
** DESCRIPTION:
**    Return a count of modification operations against a DOM. This
**    may be used by a caller to detect whether a DOM has been changed
**    since a prior call to the interface. For example, a caller may
**    use a change in the modification count to detect that an in-progress
**    iteration over field or array items may be invalidated.
**
** NOTES:
**   This interface will typically return 0 (for DOMs, such as OSON, that
**   are read-only), or 1 for DOMs that have not been modified beyond setting
**   the root node after creation (e.g. from an event source). Otherwise,
**   the value monotonically increases for the life of the DOM, until
**   reset or reused.
*/
#define JsonDomGetModifications(jdoc) \
  ((jdoc)->modcount_jznDomDoc)

/*
 ** NAME:
 **   JsonDomNodeVisitor
 **
 ** PARAMETERS:
 **  jznDomDoc      *jdoc     (IN)  - DOM doc to be visited
 **  jznDomNode     *node     (IN)  - starting node to be visited
 **  JsonDomVisitorFunc func  (IN)  - callback func user supplied
 **  void           *appctx   (IN)  - callback context user supplied
 **
 ** RETURNS:
 **   void
 **
 ** DESCRIPTION:
 **    walks through each DOM node and calls a user supplifed api function
 **    <func> with api specific application context <ctx>
 */
#define JsonDomNodeVisitor(jdoc, node, func, appctx) \
  ((jdoc)->mthds_jznDomDoc->jznDomNodeVisitor((jdoc),(node),(func),(appctx)))

/*
** NAME:
**   JsonDomGetFieldPosition
**
** PARAMETERS:
**   jznDomDoc       *jdoc   (IN)  - JSON DOM
**   jznDomObject    *obj    (IN)  - Object node within the DOM
**   oratext         *fname  (IN)  - Field name string
**   ub2              fnlen  (IN)  - Length of field name string in bytes
**   ub4             *pos    (OUT) - Position within object (optional)
**
** RETURNS:
**   (jznDomNode *) the value of the field (may be any JSON node type)
**
** DESCRIPTION:
**   Returns the value of a field within the object, retrieving it by matching
**   the name string and length.
**   Returns NULL if a matching field cannot be found.
**
** NOTES:
**   Optionally returns the position of the field within the object's
**   children, equivalent to the position within a batch get sequence.
**   This will be UB4MAXVAL if the value is unavailable.
**
** ERROR HANDLING: It may return NULL in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/
#define JsonDomGetFieldPosition(jdoc, obj, fname, fnlen, pos) \
  ((jdoc)->mthds_jznDomDoc->jznDomGetFieldPos((jdoc),(obj), \
                                              (fname),(fnlen),(pos)))

/*
** NAME:
**   JsonDomSortArray
**
** PARAMETERS:
**   JsonDomDoc    *doc      (IN)  - DOM document
**   JsonDomArray  *arr      (IN)  - array node to sort
**   uword          flags    (IN)  - sort mode flags
**
** RETURNS:
**   JSON status code
**
** DESCRIPTION:
**   Sort the nodes within an array using AnyType comparisons.
**   Flags govern the type of re-arrangement that is performed:
**     - Unique     : remove duplicates, doesn't re-order the elements
**     - Unlink     : unlink duplicates instead of reporting them
**     - Delete     : delete duplicates instead of reporting them
**     - Ascending  : re-order based on collation comparisons
**     - Descending : re-order based on collation comparisons
**
** NOTES:
**   The sort is inherently an order(n * log(n)) operation.
*/
#define JsonDomSortArray(jdoc, arr, flags) \
  ((jdoc)->mthds_jznDomDoc->jznDomSortArray((jdoc),(arr),(flags)))

#define JZNDOM_ARRAY_SORT_UNIQUE      0x0001
#define JZNDOM_ARRAY_SORT_UNLINK      0x0002  /* Unlink duplicates */
#define JZNDOM_ARRAY_SORT_DELETE      0x0004  /* Delete duplicates */
#define JZNDOM_ARRAY_SORT_COUNT       0x0008  /* Count duplicates  */
#define JZNDOM_ARRAY_SORT_ASCENDING   0x0010
#define JZNDOM_ARRAY_SORT_DESCENDING  0x0020

/*
** NAME:
**   JsonDomCastValue
**
** PARAMETERS:
**   JsonDomDoc      *doc      (IN)  - DOM document
**   JsonDomScalar   *snode    (IN)  - Scalar node in the DOM to be cast
**   jznExtnType      vtype    (IN)  - Destination type to cast scalar to
**
** RETURNS:
**   JSON status code
**
** DESCRIPTION:
**   
**
** NOTES:
**   Supported only for immutable DOM.
*/
#define JsonDomCastValue(jdoc, snode, vtype) \
  ((jdoc)->mthds_jznDomDoc->jznDomCastValue((jdoc), (snode), (vtype)))


/*
** NAME:
**   JsonDomClearWF
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   ub4            flag     (IN)  - flag
**
** RETURNS:
**   (void)
**
** DESCRIPTION:
**   Clears all state associated with a DOM, returning it to the "new"
**   state and eliminating the root node (if any) and all descendants,
**   as well as all orphan nodes.
**
** NOTES:
**   This allows a DOM to be a reusable container into which a series
**   of serialized JSON documents may be loaded, processed, and unloaded.
*/
/* flag passed into JsonDomClearWF() */
#define JSONDOM_CLR_NO_CLOSE_INPUT 0x1 
/* flag relevant for oson dom to avoid closing the input oson for re-use*/

#define JsonDomClearWF(jdoc, flag) \
  ((jdoc)->mthds_jznDomDoc->jznDomDocResetWF(jdoc, flag))


/*
** NAME:
**   JsonDomDecodeOSONBytesIntoNode
**
** PARAMETERS:
**   jznDomDoc     *jdoc          (IN)  - JSON DOM
**   ub1           *oson_byte     (IN)  - OSON byte array[]
**   ub4            oson_byte_len (IN)  - OSON byte array length 
**   jsonerr       *ojerr         (OUT) - returned, error loading oson bytes
**   jznDomDoc    **out_jdoc      (OUT) - OSON DomDoc loaded from the oson bytes
**
** RETURNS:
**   (jznDomNode *) 
**
** DESCRIPTION:
**   This method decodes the input OSON bytes and returns the OSON DOM Node.
**   The OSON decoder is cached inside jdoc for future use.
**   It is used to decode oson bytes embedded as raw.
**
** NOTES:
*/
#define JsonDomDecodeOSONBytesIntoNode(jdoc, oson_byte, oson_byte_len, \
                                       ojerr, out_jdoc) \
  ((jdoc)->mthds_jznDomDoc->jznDomDocDecodeOSONBytesIntoNode \
                            (jdoc, oson_byte, oson_byte_len, ojerr, out_jdoc))

/* Bug37204122: dom api to push down range cmp/in-clause */
/*
** NAME:
**   JsonDomAryFilter
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *jary     (IN)  - JSON Node Array 
**   jznEng_RngCmpInf *filter(IN)  - Filter Spec
**
** RETURNS:
**   boolean: TRUE if evauation is successful, the filter
**             result is in the filter spec
**    FALSE if evauation fails.
**
** DESCRIPTION:
** evaluate range comp filter over array of elements to see if any array
** element of scalar value satisifies the filter
**
** NOTES:
*/
#define JsonDomAryFilter(jdoc, jary, filter) \
  ((jdoc)->mthds_jznDomDoc->jznDomAryFilter(jdoc, jary, filter))

/*
** NAME:
**   JsonDomScalarFilter
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *jary     (IN)  - JSON Node Array
**   jznEng_RngCmpInf *filter(IN)  - Filter Spec
**
** RETURNS:
**   boolean: TRUE if evauation is successful, the filter
**             result is in the filter spec
**    FALSE if evauation fails.
**
** DESCRIPTION:
** evaluate range comp filter over the scalar node to see if the scalar
** node of the scalar value satisifies the filter
**
** NOTES:
*/
#define JsonDomScalarFilter(jdoc, jsca, filter) \
  ((jdoc)->mthds_jznDomDoc->jznDomScalarFilter(jdoc, jsca, filter))

/*
** NAME:
**   JsonDomAryInClauseMrgSort
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *jary     (IN)  - JSON Node Array
**   jznEng_RngCmpInf *incI(IN)  - in clause Spec
**
** RETURNS:
**   boolean: TRUE if evauation is successful, the inclause eval
**             result is in the output variable *rslt
**    FALSE if evauation fails.
**
** DESCRIPTION:
** evaluate in clause path expr over array of sorted elements using preSorted
*** merge join eval approach.
*** Note the input array node of scalar values is sorted.
**
** NOTES:
*/
#define JsonDomAryInClauseMrgSort(jdoc, jary, incI, rslt) \
  ((jdoc)->mthds_jznDomDoc->jznDomAryInClauseMrgSort(jdoc, jary, incI, rslt))

/*
** NAME:
**   JsonDomAryInClauseBinSrch
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *jary     (IN)  - JSON Node Array
**   jznEng_RngCmpInf *incI(IN)  - in clause Spec
**
** RETURNS:
**   boolean: TRUE if evauation is successful, the inclause eval
**             result is in the output variable *rslt
**    FALSE if evauation fails.
**
** DESCRIPTION:
** evaluate in clause path expr over sorted inlist using binary search
*** Note the input array node of scalar values is NOT sorted.
**
** NOTES:
*/
#define JsonDomAryInClauseBinSrch(jdoc, jary, incI, rslt) \
  ((jdoc)->mthds_jznDomDoc->jznDomAryInClauseBinSrch(jdoc, jary, incI, rslt))


/*
** NAME:
**   JsonDomAryMinMaxJt
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *jary     (IN)  - JSON Node Array
**    jznpFunction  *fdesc   (IN)  - capture  JZNL_MIN or JZNL_MAX
**           jznlitid       funcType (IN)  - JZNL_MIN or JZNL_MAX
**
** RETURNS:
**   the array element node whose node is smallest (JZNL_MIN)
**    or JZNL_MAX using
**    json datatype range comparison (DeepDomNodesCompare) semantics
**
** DESCRIPTION:
**  given a json array, it finds the min or max of the element node using
** json datatype range comparison (DeepDomNodesCompare) semantics
**
** ERROR HANDLING: It may return JZNDOM_SCALAR in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/


#define JsonDomAryMinMaxJt(jdoc, jary, fdesc)\
  ((jdoc)->mthds_jznDomDoc->jznDomAryMinMaxJt(jdoc, jary, fdesc))


/*
** NAME:
**   JsonDomAryAggRetSca
**
** PARAMETERS:
**   jznDomDoc     *jdoc     (IN)  - JSON DOM
**   jznDomArray   *jary     (IN)  - JSON Node Array
**    jznpFunction  *fdesc   (IN)  - capture   jznlitid       funcType
**           jznlitid       funcType (IN)  - JZNL_SUM,JZNL_AVG,JZNL_MINNUMBER,
**             JZNL_MAXNUMBER, JZNL_MINSTRING, JZNL_MAXSTRING,
**             JZNL_MINDATETIME , JZNL_MAXDATETIME, 
**             JZNL_VARIANCE, JZNL_STDDEV, JZNL_STDDEVP, JZNL_COUNT 
**             see JZN_PCODE_AGG_FUNC_RET_SVAL() macro
**   ub4 modeFlags (IN)  - Agg Mode control flags
**   ub4 maxlen, output buffer max length of (*valptr_o)
**   (OUT) - aggregation value output with  output jtype_o, valptr_o, vallen_o
**   jznvaltype* jtype_o (OUT)
**   ub1 *valptr_o (OUT)
**   ub4 * vallen_o (OUT)
**
** RETURNS:
**   error code, JZNERR_OK means successful
**
** DESCRIPTION:
**  given a json array, it runs agg functio to return a scalar value
**
** ERROR HANDLING: It may return JZNDOM_SCALAR in case of error with errrcode
** obtaineable via JsonDomGetError(jdoc)
*/

#define JsonDomAryAggRetSca(jdoc,jary,fdesc, modeFlags,maxlen,jtype_o,valptr_o,\
                            vallen_o)\
  ((jdoc)->mthds_jznDomDoc->jznDomAryAggRetSca(jdoc, jary, fdesc, modeFlags,\
    maxlen,jtype_o, valptr_o, vallen_o))

/*
** NAME:
**   JsonDomHash
**
** PARAMETERS:
**   JsonDomDoc      *doc     (IN)  - DOM document
**   JsonDomNode     *node    (IN)  - starting node to be hashed
**   ub1             *outbuf  (OUT) - result of the hash
**   ub4             *outlen  (OUT) - size of hash in bytes
**
** RETURNS:
**   JSON status code
**
** DESCRIPTION:
**   Computes a canonical hash for the DOM starting from a node
**   (usually the root). Object fields are canonically ordered.
**   Scalars are turned into canonical forms (number, date/time).
**   outbuf is assumed large enough to hold the resulting hash (32 bytes).
**
** NOTES:
**   This operates slightly differently for scalar nodes than the
**   jznuToAnyType() function. They are hashed as nodes with a node type
**   code. The unbounded values are not truncated but fully hashed
**   (string, binary). Numeric types are converted to string format
**   before hashing to avoid precision/scale loss that may occur with
**   OraNum conversions. The ID types are treated as JZNVAL_ID, to
**   distinguish them from RAW binary values. Only date/time/interval
**   types are treated exactly the same.
*/
sword JsonDomHash(JsonDomDoc *doc, JsonDomNode *node,
                  ub1 *outbuf, ub4 *outlen);

#define JZNDOM_NODE_HASH_LENGTH  16 /* 128-bit output */

/*
** NAME:
**   JsonDomAnyType
**
** PARAMETERS:
**   JsonDomDoc    *doc       (IN)  - DOM document
**   JsonDomNode   *node      (IN)  - starting node to be indexed
**   ub1           *outbuf    (OUT) - result comparable image
**   ub4           *outlen    (OUT) - size of image in bytes
**   ub4            max_depth (IN)  - beyond this depth use a hash
**   ub4            max_size  (IN)  - larger containers use a hash
**   uword          flags     (IN)  - we love our flags
**
** RETURNS:
**   JSON status code
**
** DESCRIPTION:
**   Computes a memcmp-friendly image of a DOM node, similar to
**   jznuAnyScalar but working on any type of DOM node including
**   objects and arrays. Objects are stored with a start-container
**   and end-container bookends, and fields are stored as field/value
**   pairs with a special code for start-of-field-name. Fields names
**   are just appended follwed by a [0x00,0xFF] terminator. The field
**   names are sorted into a canonical order. Arrays are stored by just
**   traversing their elements in linear order and appending them,
**   however the end-of-array is marked with an end-container bookmark
**   to ensure that duplicate-key situations don't arise.
**
** NOTES:
**   Scalars are subjected to jznuAnyScalar. Fixed-length values
**   are stored as-is (date/time values are 0-padded to make them
**   11 bytes). Strings are stored as-is with a [0x00,0xFF] terminator.
**   Binaries including IDs are rounded to the nearest multiple of
**   16 bytes in size, then a length byte with the number of 16-byte
**   units (0-255 chunks, e.g. 0-4080 bytes) prepended to the payload.
**   Numbers are converted to Oracle Numbers and a 0x00 appended.
**   Pass UB4MAXVAL for max_depth and max_size to get an unlimited
**   traversal. Passing 0 for the max_size will force all objects/arrays
**   including the start node to be similar to the hash of a node.
**   Passing a 0 for max_depth causes all nodes to be rendered with a hash.
*/
sword JsonDomAnyType(JsonDomDoc *doc, JsonDomNode *node,
                     ub1 *outbuf, ub4 *outlen,
                     ub4 max_depth, ub4 max_size, uword flags);

#define JZNANY_FLAG_NUMBER_EXTENDED 0x0100  /* Merge Decimal128 values       */
#define JZNANY_FLAG_NULL_ON_EMPTY   0x0200  /* Treat empty values as nulls   */
#define JZNANY_FLAG_UTF16_STRINGS   0x0400  /* Strings are in UTF16 format   */
#define JZNANY_FLAG_UTF8_STRINGS    0x0800  /* Strings are in UTF8 format    */
#define JZNANY_FLAG_JSON_STRINGS    0x0C00  /* No string conversions         */

#define JZNANY_FLAG_MATCH_NODE_TYPE 0x1000  /* Require matching node types   */
#define JZNANY_FLAG_MATCH_DATA_TYPE 0x2000  /* Require matching scalar types */
#define JZNANY_FLAG_PRESERVE_TYPES  0x4000  /* Preserve scalar types         */
#define JZNANY_FLAG_PRESERVE_ORDER  0x8000  /* Preserve field order          */

/*
** This is similar to JsonDomAnyType, but instead of rendering an image,
** this performs an on-the-fly render-and-compare against an existing
** image <buf> of length <buflen>. The result of the comparison is returned
** in <result>. Returns 0 if the values are the same, > 0 if <node> is
** greater than the old image, < 0 if <node> is less than the old image.
** The status is JZNERR_OK, or an error if the operation failed (e.g.
** due to out-of-memory conditions, type conversion failures, etc.).
*/
sword JsonDomAnyCompare(JsonDomDoc *doc, JsonDomNode *node,
                        ub1 *buf, ub4 buflen, sb4 *result,
                        ub4 max_depth, ub4 max_size, uword flags);

typedef struct jznAnyTypeReader
{
  ub1            *buf;                   /* Pointer to serialized image */
  ub4             buflen;                /* Length of serialized image */
  ub4             offset;                /* Offset of reader in image buffer */
  uword           depth;                 /* Recursive container depth */
  uword           flags;                 /* Mode flags */
  ub1             arr[JZN_MAX_DEPTH];    /* Container types */
  ub1             tmp[JZN_BINFMT_LEN];   /* Staging buffer */
  ub1             dummy[16];             // ### Remove later
  jznEventRecord  rec;                   /* Most recent event (OUT) */
} jznAnyTypeReader;

/*
** Reads an event from an anyNode binary image. The current state of the
** reader is maintained in a reader context structure exposed to the caller
** to avoid the need for heap allocations. This structure should be
** initialized to point to the image/length, and the read offset to 0
** before starting to read. The flags should be set to the same as
** they were for the encoder, especially the value of the numeric padding.
*/
sword JsonDomReadAnyType(jznAnyTypeReader *reader);

/*--------------------------- Deprecated interfaces -------------------------*/

/*
** Users are not encouraged to use the following deprecated interfaces.
*/

/*
** NAME:
**   JsonDomLoadAndEncodeImage
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN)  - document to load events into
**   jznEventSource *evtsrc  (IN)  - event source (such as the JSON parser)
**   jznosonFile    *ofile   (OUT) - OSON file (or NULL)
**
** RETURNS:
**   (jznDomNode *) the root node of the DOM, or NULL on error.
**
** DESCRIPTION:
**   After constructing an empty jznDomDoc, it can be loaded from an existing
**   object in two ways.
**
**   When jznDomDoc is JZN_INMEM_DOM type, this method loads a JSON document
**   input from a JSON event stream (typically the JSON parser, or a source
**   such as the BSON decoder). The ofile parameter can be ignored.
**
**   When jznDomDoc is JZN_OSON_DOM type, this method encodes the event
**   stream in OSON serialized format and writes the bytes to a file stream
**   (the ofile parameter)
**
** NOTES:
**   Always check the error code by calling JsonDomGetGetError before proceeding.
**
** ### Legacy interface for compatibility.
*/
jznDomNode *jznDomLoadAndEncodeImage(jznDomDoc *jdoc,
                                     jznEventSource *evtsrc,
                                     jznosonFile *ofile);

/*
** NAME:
**   JsonDomGetXmlContext
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**
** RETURNS:
**   (xmlctx *) the XML context underlying this DOM
**
** DESCRIPTION:
**   Get the original enviroment context, xmlctx that was used
**   to create this jznDomDoc.
*/
#define JsonDomGetXmlContext(jdoc) \
  ((jdoc)->xctx_jznDomDoc)

/*
** NAME:
**   JsonDomSetXmlContext
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**   xmlctx         *xctx    (IN) - New XML context to use for DOM operations
**
** RETURNS:
**   (void)
**
** DESCRIPTION:
**   Set (override) the XML context for this DOM.
*/
void JsonDomSetXmlContext(jznDomDoc *jdoc, xmlctx *xctx);

/*
** NAME:
**   JsonDomGetUserContext
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**
** RETURNS:
**   (void *)
**
** DESCRIPTION:
**   Return the user context for this DOM, or NULL if none has been set.
*/
void *JsonDomGetUserContext(JsonDomDoc *doc);

/*
** NAME:
**   JsonDomSetUserContext
**
** PARAMETERS:
**   jznDomDoc      *jdoc    (IN) - JSON DOM
**   void           *usrctx  (IN) - User context pointer
**
** RETURNS:
**   (void)
**
** DESCRIPTION:
**   Set (override) the user defined context pointer for this DOM.
*/
void  JsonDomSetUserContext(JsonDomDoc *doc, void *usrctx);

/*
** Internal interface for unwrapping a document that is an array
** with a single element. Returns JZNERR_OK if successful.
*/
sword jznDomUnwrapRootArray(JsonDomDoc *doc);

#define jznDomM_LoadFromInputEventSrc(jdoc, evtsrc, ofile) \
  jznDomLoadAndEncodeImage((jdoc), (evtsrc), (ofile))

#define jznDomM_LoadFromInputOSON         JsonDomLoadImage
#define jznDomM_GetEnvCtx                 JsonDomGetXmlContext

#define jznDomM_GetNodeType               JsonDomGetNodeType
#define jznDomM_GetScalarInfo             JsonDomGetScalarInfo
#define jznDomM_GetRootNode               JsonDomGetRoot
#define jznDomM_GetNumObjField            JsonDomGetNumFields
#define jznDomM_GetFieldVal               JsonDomGetFieldValue
#define jznDomM_GetFieldByName            JsonDomGetFieldByName
#define jznDomM_GetAllFieldNamesAndVals   JsonDomGetAllFields
#define jznDomM_GetFieldNamesAndValsBatch JsonDomGetFields
#define jznDomM_GetArraySize              JsonDomGetArraySize
#define jznDomM_GetArrayElem              JsonDomGetArrayElement
#define jznDomM_GetArrayElemBatch         JsonDomGetElements

#define jznDomM_SetRootNode               JsonDomSetRoot
#define jznDomM_PutFieldValue             JsonDomSetField
#define jznDomM_PutItem                   JsonDomAddElement
#define jznDomM_AppendItem                JsonDomAppendElement
#define jznDomM_ReplaceItem               JsonDomReplaceElement
#define jznDomM_DeleteField               JsonDomDeleteField
#define jznDomM_UnlinkField               JsonDomUnlinkField
#define jznDomM_RenameField               JsonDomRenameField
#define jznDomM_DeleteItem                JsonDomDeleteElement
#define jznDomM_UnlinkItem                JsonDomUnlinkElement
#define jznDomM_DeleteItemBatch           JsonDomDeleteElementRange

#define jznDomM_NewObject                 JsonDomCreateObject
#define jznDomM_NewArray                  JsonDomCreateArray
#define jznDomM_NewScalar                 JsonDomCreateScalar

#define jznDomM_DocReset                  JsonDomClear
#define jznDomM_DocFree                   JsonDomFree
#define jznDomM_Equals                    JsonDomEquals
#define jznDomM_Copy                      JsonDomCopy
#define jznDomM_GetOutputEventSrc         JsonDomToEventSource
#define jznDomM_ValidFid                  JsonDomCheckFieldKeys
#define jznDomM_Print                     JsonDomPrintDocument
#define jznDomM_GetError                  JsonDomGetError

/* ### DEPRECATED interface used by clsjson.c ### */
#define jznDomM_SetError(doc, err) \
  ((doc)->errcode_jznDomDoc = (err))

/* ### DEPRECATED interface used by clsjson.c ### */
#define jznDomM_PrintNode(jd, pc, nd) \
  JsonDomPrintNode((jd),(nd),jznuPrintToWriter((pc)))

/* ### DEPRECATED interface, drop it later ### */
#define jznDomM_SerializeNode(jd, wt, nd) \
  JsonDomPrintNode((jd),(nd),(wt))

#ifdef JUST_AN_EXAMPLE
/*
 * Following is an example to show how to use JSON DOM APIs to
 * write a recursive function jznDomPrint() that navigates the
 * JSON DOM tree to print it into JSON text.
 */
STATICF inline jsonerr jznCheckReportErr(JsonDomDoc *doc)
{
  jsonerr err;
  oratext *msg;
  if ((err = JsonDomGetError(doc)) != JZNERR_OK)
  {
    msg = jznDomGetErrorMessage(doc, &err, TRUE);
    printf("%.*s", (char *)msg);
    return err;
  }
  else
    return JZNERR_OK; /* no error */
}
STATICF jsonerr jznDomPrint(JsonDomDoc *doc, JsonDomNode *node)
{
  JsonDomNode *np;
  jznnodetype  ndt = JsonDomGetNodeType(doc, node);
  ub4          sz, i;
  jsonerr      err;

  switch (ndt)
  {
    case JZNDOM_SCALAR:
    {
     jznScalarVal val;
     oratext      buf[JZN_SCALAR_VAL_SZ];
     ub4          buflen = (ub4)sizeof(buf);

     JsonDomGetScalarInfo(doc, (JsonDomScalar *)node, &val);

     err = jznCheckReportErr(doc);
     if (err != JZNERR_OK)
      return err;

     jznuScalarToString(JsonDomGetXmlContext(doc),
                        val.type_jznScalarVal,
                        &val.uval_jznScalarVal,
                        buf, &buflen);

     printf("%.*s", (int)buflen, buf);
    }
    break;
    case JZNDOM_OBJECT:
    {
      jznDomNameValuePair *fnma;
      ub4 sz = JsonDomGetNumFields(doc, (JsonDomObject *)node);

      err = jznCheckReportErr(doc);
      if (err != JZNERR_OK)
        return err;

      fnma = (jznDomNameValuePair *)malloc(sz * sizeof(*fnma));

      JsonDomGetAllFields(doc, (JsonDomObject *)node, fnma);


      err = jznCheckReportErr(doc);
      if (err != JZNERR_OK)
        return err;

      printf("{");
      for (i = 0; i < sz; ++i)
      {
        jznDomObjFieldName *nmp = &(fnma[i].name_jznDomNameValuePair);
        np = JsonDomGetFieldValue(doc, (JsonDomObject *)node, nmp);

        err = jznCheckReportErr(doc);
        if (err != JZNERR_OK)
          return err;

        if (i > 0)
          printf(",");
        printf("\"%.*s\":", nmp->len_jznDomObjFieldName,
                            nmp->fnm_jznDomObjFieldName);
        err = jznDomPrint(doc, np);
        if (err != JZNERR_OK)
          return err;
      }
      printf("}");

      free(fnma);
    }
    break;
    case JZNDOM_ARRAY:
    {
        printf("[");
#ifdef SINGLE_ARRAY_API
      {
        ub4 sz = JsonDomGetArraySize(doc, (JsonDomArray *)node);

        err = jznCheckReportErr(doc);
        if (err != JZNERR_OK)
          return err;

        for (i = 0; i < sz; ++i)
        {
          np = JsonDomGetArrayElement(doc, (JsonDomArray *)node, i);

          err = jznCheckReportErr(doc);
          if (err != JZNERR_OK)
            return err;

          err = jznDomPrint(doc, np);
          if (err != JZNERR_OK)
            return err;

          if (i != (sz -1))
            printf(",");
        }
     }
#else
    {
      // use array batch fetch api, shall be more efficient
      JsonDomNode *ndAry[256];
      ub4          startpos = 0;
      ub4          numfetched = (ub4)(sizeof(ndAry)/sizeof(*ndAry));
      boolean      first = TRUE;

      while (numfetched = JsonDomGetElements(doc, (JsonDomArray *)node,
                                             startpos, numfetched, ndAry))
      {
        err = jznCheckReportErr(doc);
        if (err != JZNERR_OK)
            return err;

        for (i = 0; i < numfetched; ++i)
        {
          if (!first)
            printf(",");
          else
           first = FALSE;

          np = ndAry[i];
          err = jznDomPrint(doc, np);
          if (err != JZNERR_OK)
            return err;
        }
        // increment startpos
        startpos += numfetched;
      }
    }
#endif
        printf("]");
    }
    break;
    default:
    break;
    return JZNERR_OK;
  }
}
#endif /* JUST_AN_EXAMPLE */

/*****************************************************************************
 * DOM Event writer interface - write events to a builder to create a DOM.
 *****************************************************************************/

/*
** Stateful DOM builder/writer. This object can be used to
** build a DOM from events.
*/
typedef struct jznDomWriter jznDomWriter;

/*
** Create a stateful context to serve as DOM builder from events.
** This uses memory from the input XML context.
*/
jznDomWriter *jznDomWriterCreate(xmlctx *xctx);

/*
** Create writer with dedicated memory.
*/
jznDomWriter *jznDomWriterCreateEx(xmlctx *xctx, oramemctx *omctx);

/*
** Destroy a stateful DOM writer.
*/
void jznDomWriterDestroy(jznDomWriter *writer);

/*
** Set a specific DOM document to be the writer destination
** This is useful if using a writer to copy a set of events
** into an existing DOM. Use jznWriterGetTree to get the result.
*/
sword jznDomWriterSetDoc(jznDomWriter *writer,
                         JsonDomDoc *doc, ub4 flags);

/*
** Clear and initialize the writer for a new operation.
** This is necessary if the prior operation wasn't completed or
** ended in an error condition.
** If a target document remained from the prior operation, it's returned
** and the writer's internal target is cleared.
*/
jznDomDoc *jznDomWriterClear(jznDomWriter *writer);

/*
** Get a handle to an event writer from the stateful DOM writer.
*/
jznEventWriter *jznDomWriterGetSink(jznDomWriter *writer);


/*
** The flags that are relevant are DOM-related creation flags
*/
void jznDomWriterSetFlags(jznDomWriter *ctx, ub4 flags);

/*
** Get the count of events written to the DOM under construction
** from the DOM writer.
*/
ub4 jznDomWriterGetWrtCnt(jznDomWriter *writer);

/*
** Write a textual JSON into the target DOM for this writer.
** The text needs to be in a Unicode format (UTF-8 or UTF-16).
** The length is in bytes.
** This is useful as a convenience for creating a DOM from a string.
*/
sword jznDomWriterParse(jznDomWriter *writer, void *buf, ub4 buflen);

/*
** Consume events from a source and send them to the DOM writer.
** This interface will stop when it has consumed an entire JSON
** item, such as an object or array. It will not read until the
** JZNEV_END event occurs. It can therefore be used to read/load
** sub-objects.
*/
sword jznDomWriterReadSource(jznDomWriter *writer,
                             jznEventSource *eventSource);

/*
** Consume all events remaining in a source and put them to the writer.
** This interface continues to read until a JZNEV_END event is delievered.
** It's designed to load an entire JSON document.
*/
sword jznDomWriterReadAll(jznDomWriter *writer,
                          jznEventSource *eventSource);

/*
** Returns the DOM node at the root of the subtree built by this writer.
** The caller is assumed to have the document, e.g. by calling
** jznDomWriterSetDoc() first.
** Returns NULL if there's an error or if the tree is incomplete.
*/
JsonDomNode *jznDomWriterGetTree(jznDomWriter *writer);

/*
** Returns the DOM built by this writer.
** Internally, the root node is set to the top of the tree built
** by the events, and the document is detached and returned from
** the writer.
** Returns NULL if there's an error or if the DOM is incomplete.
*/
JsonDomDoc *jznDomWriterBuild(jznDomWriter *writer);

/*
** Returns the orphan node subtree built by this writer.
** The document is detached from the writer internally;
** the caller is assumed to have a reference to it.
*/
JsonDomNode *jznDomWriterBuildTree(jznDomWriter *writer);

/*
** Get the document under construction, if any. This returns
** the document node even if it's incomplete and/or hasn't yet
** had the root set (which Build does). Leaves the in-process
** construction unchanged. Internal use only!
*/
JsonDomDoc *jznDomWriterGetDoc(jznDomWriter *writer);

/*
** Get a count of events written so far to the writer.
** In the reset state, this is 0.
*/
ub4 jznDomWriterGetCount(jznDomWriter *writer);

/*
** Release a DOM back to the writer for reuse.
*/
void jznDomWriterRelease(jznDomWriter *writer, JsonDomDoc *doc);

void jznDomWriterSetXmlCtx(jznDomWriter *writer, xmlctx *xctx);

/*****************************************************************************
 * DOM Event source interface - pull events from a DOM node tree
 *****************************************************************************/

/*
** Stateful DOM reader/eventer.
** This object can be used to set up an event source starting from
** any node in the DOM. This object is stateful; it can provide one
** event source at a time. The source is backed by the DOM, which
** should not be changed during event production.
*/
typedef struct jznDomReader jznDomReader;

/*
** Create a reader context to serve as source of DOM events.
** This uses memory from the input XML context.
*/
jznDomReader *jznDomReaderCreate(xmlctx *xctx);

/*
** Create reader with dedicated memory.
*/
jznDomReader *jznDomReaderCreateEx(xmlctx *xctx, oramemctx *omctx);

/*
** Set up a handle to an event source given a DOM and optional start node.
** If the input node is NULL, events start at the DOM root node.
*/
jznEventSource *jznDomReaderInit(jznDomReader *reader,
                                 JsonDomDoc *doc, JsonDomNode *node);

/*
** Destroy a DOM reader.
*/
void jznDomReaderDestroy(jznDomReader *reader);

/*
** Set mode flags such as:
**   JZN_USE_EXTENSION_TYPES
*/
void jznDomReaderSetFlags(jznDomReader *reader, ub4 flags);

void jznDomReaderSetXmlCtx(jznDomReader *reader, xmlctx *xctx);

/*
** SAX traversal of a DOM from a given node
*/
sword JsonSaxDom(JsonDomDoc *doc, JsonDomNode *node,
                 JsonSaxHandler cb, void *cbctx);

/*
** Render the JSON column modifier <jtmf> as a string.
** Output string is to <jtms>[<jtmslen>] up to <maxlen> bytes.
*/
jsonerr jznDomRenderModifier(jznComplexAttributes *jtmf,
                             char *jtms, ub4 *jtmslen, ub4 maxlen,
                             boolean check, boolean modifierOnly);

void jznDom_jtmStr(jznComplexAttributes *jtmf,
                   jsonerr *errp, char *jtms, ub4 *jtmslen, ub4 maxlen,
                   boolean check, boolean modifierOnly);

/*
** JSON type modifier check against DOM 
**  bug fix 36817752
*/
#define JZNDOM_CHK_STR_SZ 128
typedef struct jznDomModChkCx
{
  jznComplexAttributes  mod_jznDomModChkCx;
  char                 *str_jznDomModChkCx;
  ub4                   len_str_jznDomModChkCx;
  ub4                   osonlim_jznDomModChkCx; /* oson size limit */
  char                  inst_jznDomModChkCx[JZNDOM_CHK_STR_SZ];
  ub1                   flag_jznDomModChkCx; /* input flag */
#define JZNDOMCHK_ALLOW_SAME_FAM 0x01
  /* array of same scalar value family is allowed. for example, all numbers
   * in number family, all date, timestamp, timeestamp with timezone are
   * allowed during json type modifier check
   */

  ub1                   oflag_jznDomModChkCx; /* output flag */
#define JZNDOMCHK_SORT_FAILED        0x01
   /* array element value is out of order (not sorted) */
#define JZNDOMCHK_SORT_FAILED_ONLY    0x2
   /* the only failure is due to array element value is out of order */
  ub1                   spare_jznDomModChkCx[6];
} jznDomModChkCx;

#define JZNDOM_INIT_MODCHK(ctxp)\
do{\
  CLRSTRUCT(ctxp->mod_jznDomModChkCx);\
  ctxp->str_jznDomModChkCx = NULL;\
  ctxp->len_str_jznDomModChkCx = 0;\
  ctxp->osonlim_jznDomModChkCx = 0;\
  ctxp->inst_jznDomModChkCx[0];\
  ctxp->inst_jznDomModChkCx[1];\
  ctxp->flag_jznDomModChkCx = 0;\
  ctxp->oflag_jznDomModChkCx = 0;\
  memset(ctxp->spare_jznDomModChkCx, 0, 6);\
} while (0)

/* check json modifier against a DOM */
jsonerr jznDomChkJModifier(JsonDomDoc *srcDoc, JsonDomNode *node,
                          jznDomModChkCx *mod);

#endif                                              /* JZNDOM_ORACLE */
