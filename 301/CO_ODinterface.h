/**
 * CANopen Object Dictionary interface
 *
 * @file        CO_ODinterface.h
 * @ingroup     CO_ODinterface
 * @author      Janez Paternoster
 * @copyright   2020 Janez Paternoster
 *
 * This file is part of CANopenNode, an opensource CANopen Stack.
 * Project home page is <https://github.com/CANopenNode/CANopenNode>.
 * For more information on CANopen see <http://www.can-cia.org/>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef CO_OD_INTERFACE_H
#define CO_OD_INTERFACE_H

#include "301/CO_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup CO_ODinterface Object Dictionary interface
 * @ingroup CO_CANopen_301
 * @{
 * See @ref CO_ODinterface_operation
 */
/**
 * @defgroup CO_ODinterface_operation Operation
 * @{
 * The CANopen Object Dictionary is essentially a grouping of objects accessible
 * via the network in an ordered pre-defined fashion.
 *
 * Each object within the object dictionary is addressed using a 16-bit index
 * and a 8-bit sub-index.
 *
 * ### Terms
 * The term **OD object** means object from Object Dictionary located at
 * specific 16-bit index. There are different types of OD objects in CANopen:
 * variables, arrays and records (structures). Each OD object contains pointer
 * to actual data, data length(s) and attribute(s). See @ref OD_objectTypes_t.
 *
 * The term **OD variable** is basic variable of specified type. For example:
 * int8_t, uint32_t, float64_t, ... or just sequence of binary data with known
 * or unknown data length. Each OD variable resides in Object dictionary at
 * specified 16-bit index and 8-bit sub-index.
 *
 * The term **OD entry** means structure element, which contains some basic
 * properties of the OD object, indication of type of OD object and pointer to
 * all necessary data for the OD object. An array of OD entries together with
 * information about total number of OD entries represents Object Dictionary as
 * defined inside CANopenNode. See @ref OD_entry_t and @ref OD_t.
 *
 * ### Access
 * Application and the stack have access to OD objects via universal @ref OD_t
 * object and @ref OD_find() function, no direct access to custom structures,
 * which define Object Dictionary, is required. Properties for specific
 * OD variable is fetched with @ref OD_getSub() function. And access to actual
 * variable is via @b read and @b write functions. Pointer to those two
 * functions is fetched by @ref OD_getSub(). See @ref OD_stream_t and
 * @ref OD_subEntry_t. See also shortcuts: @ref CO_ODgetSetters, for access to
 * data of different type.
 *
 * ### Optional extensions
 * There are some optional extensions to the Object Dictionary: fixed **low and
 * high limit** prevent writing wrong value, PDO flags and IO extension.
 * **PDO flags** informs application, if specific OD variable was received or
 * sent by PDO. And also gives the application ability to request a TPDO, to
 * which variable is possibly mapped.
 * **IO extension** gives the application ability to take full control over the
 * OD object. Application can specify own @b read and @b write functions and own
 * object, on which they operate.
 *
 * ### Example usage
 * @code
extern const OD_t ODxyz;

void myFunc(const OD_t *od) {
    ODR_t ret;
    const OD_entry_t *entry;
    OD_subEntry_t subEntry;
    OD_IO_t io1008;
    char buf[50];
    OD_size_t bytesRd;
    int error = 0;

    //Init IO for "Manufacturer device name" at index 0x1008, sub-index 0x00
    entry = OD_find(od, 0x1008);
    ret = OD_getSub(entry, 0x00, &subEntry, &io1008.stream);
    io1008.read = subEntry.read;
    //Read with io1008
    if (ret == ODR_OK)
        bytesRd = io1008.read(&io1008.stream, 0x00, &buf[0], sizeof(buf), &ret);
    if (ret != ODR_OK) error++;

    //Use helper and set "Producer heartbeat time" at index 0x1008, sub 0x00
    ret = OD_set_u16(OD_find(od, 0x1017), 0x00, 500);
    if (ret != ODR_OK) error++;
}
 * @endcode
 * There is no need to include ODxyt.h file, it is only necessary to know, we
 * have ODxyz defined somewhere.
 *
 * Second example is simpler and use helper function to access OD variable.
 * However it is not very efficient, because it goes through all search
 * procedures.
 *
 * If access to the same variable is very frequent, it is better to
 * use first example. After initialization, application has to remember only
 * "io1008" object. Frequent reading of the variable is then very efficient.
 *
 * ### Simple access to OD via globals
 * Some simple user applications can also access some OD variables directly via
 * globals.
 *
 * @warning
 * If OD object has IO extension enabled, then direct access to its OD variables
 * must not be used. Only valid access is via read or write or helper functions.
 *
 * @code
#include ODxyz.h

void myFuncGlob(void) {
    //Direct address instead of OD_find()
    const OD_entry_t *entry_errReg = ODxyz_1001_errorRegister;

    //Direct access to OD variable
    uint32_t devType = ODxyz_0.x1000_deviceType;
    ODxyz_0.x1018_identity.serialNumber = 0x12345678;
}
 * @endcode
 * @} */

/**
 * @defgroup CO_ODinterface_OD_example Object Dictionary example
 * @{
 * Actual Object dictionary for one CANopen device is defined by pair of
 * OD_xyz.h and ODxyz.h files.
 *
 * "xyz" is name of Object Dictionary, usually "0" is used for default.
 * Configuration with multiple Object Dictionaries is also possible.
 *
 * Data for OD definition are arranged inside multiple structures. Structures
 * are different for different configuration of OD. Data objects, created with
 * those structures, are constant or are variable.
 *
 * Actual OD variables are arranged inside multiple structures, so called
 * storage groups. Selected groups can be stored to non-volatile memory.
 *
 * @warning
 * Manual editing of ODxyz.h/.c files is very error-prone.
 *
 * Pair of ODxyz.h/.c files can be generated by OD editor tool. The tool can
 * edit standard CANopen device description file in xml format. Xml file may
 * include also some non-standard elements, specific to CANopenNode. Xml file is
 * then used for automatic generation of ODxyz.h/.c files.
 *
 * ### Example ODxyz.h file
 * @code
typedef struct {
    uint32_t x1000_deviceType;
    uint8_t x1001_errorRegister;
    struct {
        uint8_t maxSubIndex;
        uint32_t vendorID;
        uint32_t productCode;
        uint32_t revisionNumber;
        uint32_t serialNumber;
    } x1018_identity;
} ODxyz_0_t;

typedef struct {
    uint8_t x1001_errorRegister;
} ODxyz_1_t;

extern ODxyz_0_t ODxyz_0;
extern ODxyz_1_t ODxyz_1;
extern const OD_t ODxyz;

#define ODxyz_1000_deviceType &ODxyz.list[0]
#define ODxyz_1001_errorRegister &ODxyz.list[1]
#define ODxyz_1018_identity &ODxyz.list[2]
 * @endcode
 *
 * ### Example ODxyz.c file
 * @code
#define OD_DEFINITION
#include "301/CO_ODinterface.h"
#include "ODxyz.h"

typedef struct {
    OD_extensionIO_t xio_1001_errorRegister;
} ODxyz_ext_t;

typedef struct {
    OD_obj_var_t o_1000_deviceType;
    OD_obj_var_t orig_1001_errorRegister;
    OD_obj_extended_t o_1001_errorRegister;
    OD_obj_var_t o_1018_identity[5];
} ODxyz_objs_t;

ODxyz_0_t ODxyz_0 = {
    .x1000_deviceType = 0L,
    .x1018_identity = {
        .maxSubIndex = 4,
        .vendorID = 0L,
        .productCode = 0L,
        .revisionNumber = 0L,
        .serialNumber = 0L,
    },
};

ODxyz_1_t ODxyz_1 = {
    .x1001_errorRegister = 0,
};

static ODxyz_ext_t ODxyz_ext = {0};

static const ODxyz_objs_t ODxyz_objs = {
    .o_1000_deviceType = {
        .data = &ODxyz_0.x1000_deviceType,
        .attribute = ODA_SDO_R | ODA_MB,
        .dataLength = 4,
    },
    .orig_1001_errorRegister = {
        .data = &ODxyz_1.x1001_errorRegister,
        .attribute = ODA_SDO_R,
        .dataLength = 1,
    },
    .o_1001_errorRegister = {
        .flagsPDO = NULL,
        .extIO = &ODxyz_ext.xio_1001_errorRegister,
        .odObjectOriginal = &ODxyz_objs.orig_1001_errorRegister,
    },
    .o_1018_identity = {
        {
            .data = &ODxyz_0.x1018_identity.maxSubIndex,
            .attribute = ODA_SDO_R,
            .dataLength = 1,
        },
        {
            .data = &ODxyz_0.x1018_identity.vendorID,
            .attribute = ODA_SDO_R | ODA_MB,
            .dataLength = 4,
        },
        {
            .data = &ODxyz_0.x1018_identity.productCode,
            .attribute = ODA_SDO_R | ODA_MB,
            .dataLength = 4,
        },
        {
            .data = &ODxyz_0.x1018_identity.revisionNumber,
            .attribute = ODA_SDO_R | ODA_MB,
            .dataLength = 4,
        },
        {
            .data = &ODxyz_0.x1018_identity.serialNumber,
            .attribute = ODA_SDO_R | ODA_MB,
            .dataLength = 4,
        },
    }
};

const OD_t ODxyz = {
    3, {
    {0x1000, 0, 0, ODT_VAR, &ODxyz_objs.o_1000_deviceType},
    {0x1001, 0, 1, ODT_EVAR, &ODxyz_objs.o_1001_errorRegister},
    {0x1018, 4, 0, ODT_REC, &ODxyz_objs.o_1018_identity},
    {0x0000, 0, 0, 0, NULL}
}};
 * @endcode
 * @} */


#ifndef OD_size_t
/** Variable of type OD_size_t contains data length in bytes of OD variable */
#define OD_size_t uint32_t
/** Type of flagsPDO variable from OD_subEntry_t */
#define OD_flagsPDO_t uint32_t
#endif

/** Size of Object Dictionary attribute */
#define OD_attr_t uint8_t


/**
 * Attributes (bit masks) for OD sub-object.
 */
typedef enum {
    ODA_SDO_R = 0x01, /**< SDO server may read from the variable */
    ODA_SDO_W = 0x02, /**< SDO server may write to the variable */
    ODA_SDO_RW = 0x03, /**< SDO server may read from or write to the variable */
    ODA_TPDO = 0x04, /**< Variable is mappable into TPDO (can be read) */
    ODA_RPDO = 0x08, /**< Variable is mappable into RPDO (can be written) */
    ODA_TRPDO = 0x0C, /**< Variable is mappable into TPDO or RPDO */
    ODA_TSRDO = 0x10, /**< Variable is mappable into transmitting SRDO */
    ODA_RSRDO = 0x20, /**< Variable is mappable into receiving SRDO */
    ODA_TRSRDO = 0x30, /**< Variable is mappable into tx or rx SRDO */
    ODA_MB = 0x40, /**< Variable is multi-byte (uint32_t, etc) */
    ODA_NOINIT = 0x80, /**< Variable has no initial value. Can be used with
    OD objects, which has IO extension enabled. Object dictionary does not
    reserve memory for the variable and storage is not used. */
} OD_attributes_t;


/**
 * Return codes from OD access functions
 */
typedef enum {
/* !!!! WARNING !!!! */
/* If changing these values, change also OD_getSDOabortCode() function! */
    ODR_PARTIAL        = -1, /**< Read/write is only partial, make more calls */
    ODR_OK             = 0,  /**< Read/write successfully finished */
    ODR_OUT_OF_MEM     = 1,  /**< Out of memory */
    ODR_UNSUPP_ACCESS  = 2,  /**< Unsupported access to an object */
    ODR_WRITEONLY      = 3,  /**< Attempt to read a write only object */
    ODR_READONLY       = 4,  /**< Attempt to write a read only object */
    ODR_IDX_NOT_EXIST  = 5,  /**< Object does not exist in the object dict. */
    ODR_NO_MAP         = 6,  /**< Object cannot be mapped to the PDO */
    ODR_MAP_LEN        = 7,  /**< PDO length exceeded */
    ODR_PAR_INCOMPAT   = 8,  /**< General parameter incompatibility reasons */
    ODR_DEV_INCOMPAT   = 9,  /**< General internal incompatibility in device */
    ODR_HW             = 10, /**< Access failed due to hardware error */
    ODR_TYPE_MISMATCH  = 11, /**< Data type does not match */
    ODR_DATA_LONG      = 12, /**< Data type does not match, length too high */
    ODR_DATA_SHORT     = 13, /**< Data type does not match, length too short */
    ODR_SUB_NOT_EXIST  = 14, /**< Sub index does not exist */
    ODR_INVALID_VALUE  = 15, /**< Invalid value for parameter (download only) */
    ODR_VALUE_HIGH     = 16, /**< Value range of parameter written too high */
    ODR_VALUE_LOW      = 17, /**< Value range of parameter written too low */
    ODR_MAX_LESS_MIN   = 18, /**< Maximum value is less than minimum value */
    ODR_NO_RESOURCE    = 19, /**< Resource not available: SDO connection */
    ODR_GENERAL        = 20, /**< General error */
    ODR_DATA_TRANSF    = 21, /**< Data cannot be transferred or stored to app */
    ODR_DATA_LOC_CTRL  = 22, /**< Data can't be transf (local control) */
    ODR_DATA_DEV_STATE = 23, /**< Data can't be transf (present device state) */
    ODR_OD_MISSING     = 23, /**< Object dictionary not present */
    ODR_NO_DATA        = 25, /**< No data available */
    ODR_COUNT          = 26  /**< Last element, number of responses */
} ODR_t;


/**
 * IO stream structure, used for read/write access to OD variable.
 *
 * Structure is initialized with @ref OD_getSub() function.
 */
typedef struct {
    /** Pointer to data object, on which will operate read/write function */
    void *dataObject;
    /** Data length in bytes or 0, if length is not specified */
    OD_size_t dataLength;
    /** In case of large data, dataOffset indicates position of already
     * transferred data */
    OD_size_t dataOffset;
} OD_stream_t;


/**
 * Structure describing properties of a variable, located in specific index and
 * sub-index inside the Object Dictionary.
 *
 * Structure is initialized with @ref OD_getSub() function.
 */
typedef struct {
    /** Object Dictionary index */
    uint16_t index;
    /** Object Dictionary sub-index */
    uint8_t subIndex;
    /** Maximum sub-index in the OD object */
    uint8_t maxSubIndex;
    /** Group for non-volatile storage of the OD object */
    uint8_t storageGroup;
    /** Attribute bit-field of the OD sub-object, see OD_attributes_t */
    OD_attr_t attribute;
    /** Low limit of the parameter value, not valid if greater than highLimit */
    int32_t lowLimit;
    /** High limit of the parameter value, not valid if lower than lowLimit */
    int32_t highLimit;
    /**
     * Pointer to PDO flags bit-field. This is optional extension of OD object.
     * If OD object has enabled this extension, then each sub-element is coupled
     * with own flagsPDO variable of size 8 to 64 bits (size is configurable
     * by @ref OD_flagsPDO_t). Flag is useful, when variable is mapped to RPDO
     * or TPDO.
     *
     * If sub-element is mapped to RPDO, then bit0 is set to 1 each time, when
     * any RPDO writes new data into variable. Application may clear bit0.
     *
     * If sub-element is mapped to TPDO, then TPDO will set one bit on the time,
     * it is sent. First TPDO will set bit1, second TPDO will set bit2, etc. Up
     * to 63 TPDOs can use flagsPDO.
     *
     * Another functionality is with asynchronous TPDOs, to which variable may
     * be mapped. If corresponding bit is 0, TPDO will be sent. This means, that
     * if application sets variable pointed by flagsPDO to zero, it will trigger
     * sending all asynchronous TPDOs (up to first 63), to which variable is
     * mapped. */
    OD_flagsPDO_t *flagsPDO;
    /**
     * Function pointer for reading value from specified variable from Object
     * Dictionary. If OD variable is larger than buf, then this function must
     * be called several times. After completed successful read function returns
     * 'ODR_OK'. If read is partial, it returns 'ODR_PARTIAL'. In case of errors
     * function returns code similar to SDO abort code.
     *
     * Read can be restarted with @ref OD_rwRestart() function.
     *
     * At the moment, when Object Dictionary is initialised, every variable has
     * assigned the same "read" function. This default function simply copies
     * data from Object Dictionary variable. Application can bind its own "read"
     * function for specific object. In that case application is able to
     * calculate data for reading from own internal state at the moment of
     * "read" function call. For this functionality OD object must have IO
     * extension enabled. OD object must also be initialised with
     * @ref OD_extensionIO_init() function call.
     *
     * @param stream Object Dictionary stream object, returned from
     *               @ref OD_getSub() function, see @ref OD_stream_t.
     * @param subIndex Object Dictionary subIndex of the accessed element.
     * @param buf Pointer to external buffer, where to data will be copied.
     * @param count Size of the external buffer in bytes.
     * @param [out] returnCode Return value from @ref ODR_t.
     *
     * @return Number of bytes successfully read.
     */
    OD_size_t (*read)(OD_stream_t *stream, uint8_t subIndex,
                      void *buf, OD_size_t count, ODR_t *returnCode);
    /**
     * Function pointer for writing value into specified variable inside Object
     * Dictionary. If OD variable is larger than buf, then this function must
     * be called several times. After completed successful write function
     * returns 'ODR_OK'. If read is partial, it returns 'ODR_PARTIAL'. In case
     * of errors function returns code similar to SDO abort code.
     *
     * Write can be restarted with @ref OD_rwRestart() function.
     *
     * At the moment, when Object Dictionary is initialised, every variable has
     * assigned the same "write" function, which simply copies data to Object
     * Dictionary variable. Application can bind its own "write" function,
     * similar as it can bind "read" function.
     *
     * @param stream Object Dictionary stream object, returned from
     *               @ref OD_getSub() function, see @ref OD_stream_t.
     * @param subIndex Object Dictionary subIndex of the accessed element.
     * @param buf Pointer to external buffer, from where data will be copied.
     * @param count Size of the external buffer in bytes.
     * @param [out] returnCode Return value from ODR_t.
     *
     * @return Number of bytes successfully written.
     */
    OD_size_t (*write)(OD_stream_t *stream, uint8_t subIndex,
                       const void *buf, OD_size_t count, ODR_t *returnCode);
} OD_subEntry_t;


/**
 * Helper structure for storing all objects necessary for frequent read from or
 * write to specific OD variable. Structure can be used by application and can
 * be filled inside and after @ref OD_getSub() function call.
 */
typedef struct {
    /** Object passed to read or write */
    OD_stream_t stream;
    /** Read function pointer, see @ref OD_subEntry_t */
    OD_size_t (*read)(OD_stream_t *stream, uint8_t subIndex,
                      void *buf, OD_size_t count, ODR_t *returnCode);
    /** Write function pointer, see @ref OD_subEntry_t */
    OD_size_t (*write)(OD_stream_t *stream, uint8_t subIndex,
                       const void *buf, OD_size_t count, ODR_t *returnCode);
} OD_IO_t;


/**
 * Object Dictionary entry for one OD object.
 *
 * OD entries are collected inside OD_t as array (list). Each OD entry contains
 * basic information about OD object (index, maxSubIndex and storageGroup) and
 * access function together with a pointer to other details of the OD object.
 */
typedef struct {
    /** Object Dictionary index */
    uint16_t index;
    /** Maximum sub-index in the OD object */
    uint8_t maxSubIndex;
    /** Group for non-volatile storage of the OD object */
    uint8_t storageGroup;
    /** Type of the odObject, indicated by @ref OD_objectTypes_t enumerator. */
    uint8_t odObjectType;
    /** OD object of type indicated by odObjectType, from which @ref OD_getSub()
     * fetches the information */
    const void *odObject;
} OD_entry_t;


/**
 * Object Dictionary
 */
typedef struct {
    /** Number of elements in the list, without last element, which is blank */
    uint16_t size;
    /** List OD entries (table of contents), ordered by index */
    OD_entry_t list[];
} OD_t;


/**
 * Find OD entry in Object Dictionary
 *
 * @param od Object Dictionary
 * @param index CANopen Object Dictionary index of object in Object Dictionary
 *
 * @return Pointer to OD entry or NULL if not found
 */
const OD_entry_t *OD_find(const OD_t *od, uint16_t index);


/**
 * Find sub-object with specified sub-index on OD entry returned by OD_find.
 * Function populates subEntry and stream structures with sub-object data.
 *
 * @warning If this function is called on OD object, which has IO extension
 * enabled and @ref OD_extensionIO_init() was not (yet) called on that object,
 * then subEntry and stream structures are populated with properties of
 * "original OD object". Call to this function after @ref OD_extensionIO_init()
 * will populate subEntry and stream structures with properties of
 * "newly initialised OD object". This is something very different.
 *
 * @param entry OD entry returned by @ref OD_find().
 * @param subIndex Sub-index of the variable from the OD object.
 * @param [out] subEntry Structure will be populated on success.
 * @param [out] stream Structure will be populated on success.
 *
 * @return Value from @ref ODR_t, "ODR_OK" in case of success.
 */
ODR_t OD_getSub(const OD_entry_t *entry, uint8_t subIndex,
                OD_subEntry_t *subEntry, OD_stream_t *stream);


/**
 * Verify if value written to Object Dictionary is within limit values
 *
 * @param subEntry OD sub-entry data returned by @ref OD_getSub().
 * @param val Value to be verified.
 *
 * @return Value from @ref ODR_t, "ODR_OK" in case of success.
 */
static inline ODR_t OD_checkLimits(OD_subEntry_t *subEntry, int32_t val) {
    int32_t lowLimit = subEntry->lowLimit;
    int32_t highLimit = subEntry->highLimit;

    if (lowLimit <= highLimit) {
        if (val < lowLimit) return ODR_VALUE_LOW;
        if (val > highLimit) return ODR_VALUE_HIGH;
    }
    return ODR_OK;
}


/**
 * Restart read or write operation on OD variable
 *
 * It is not necessary to call this function, if stream was initialised by
 * @ref OD_getSub(). It is also not necessary to call this function, if prevous
 * read or write was successfully finished.
 *
 * @param stream Object Dictionary stream object, returned from
 *               @ref OD_getSub() function, see @ref OD_stream_t.
 */
static inline void OD_rwRestart(OD_stream_t *stream) {
    stream->dataOffset = 0;
}


/**
 * Get SDO abort code from returnCode
 *
 * @param returnCode Returned from some OD access functions
 *
 * @return Corresponding @ref CO_SDO_abortCode_t
 */
uint32_t OD_getSDOabortCode(ODR_t returnCode);


/**
 * Initialise extended OD object with own read/write functions
 *
 * This function works on OD object, which has IO extension enabled. It gives
 * application very powerful tool: definition of own IO access on own OD
 * object. Structure and attributes are the same as defined in original OD
 * object, but data are read directly from (or written directly to) application
 * specified object via custom function calls.
 *
 * One more feature is available on IO extended object. Before application calls
 * @ref OD_extensionIO_init() it can read original OD object, which can contain
 * initial values for the data. And also, as any data from OD, data can be
 * loaded from or saved to nonvolatile storage.
 *
 * See also warning in @ref OD_getSub() function.

 * @param entry OD entry returned by @ref OD_find().
 * @param object Object, which will be passed to read or write function, must
*                not be NULL.
 * @param read Read function pointer, see @ref OD_subEntry_t.
 * @param write Write function pointer, see @ref OD_subEntry_t.
 *
 * @return true on success, false if OD object doesn't exist or is not extended.
 */
bool_t OD_extensionIO_init(const OD_entry_t *entry,
                           void *object,
                           OD_size_t (*read)(OD_stream_t *stream,
                                             uint8_t subIndex,
                                             void *buf,
                                             OD_size_t count,
                                             ODR_t *returnCode),
                           OD_size_t (*write)(OD_stream_t *stream,
                                              uint8_t subIndex,
                                              const void *buf,
                                              OD_size_t count,
                                              ODR_t *returnCode));


/**
 * Update storage group data from OD object with IO extension.
 *
 * This function must be called, before OD variables from specified storageGroup
 * will be save to non-volatile memory. This function must be called, because
 * some OD objects have IO extension enabled. And those OD object are connected
 * with application code, which have own control over the entire OD object data.
 * Application does not use original data from the storageGroup. For that reason
 * this function scans entire object dictionary, reads data from necessary
 * OD objects and copies them to the original storageGroup.
 *
 * @param od Object Dictionary
 * @param storageGroup Group of data to enable.
 */
void OD_updateStorageGroup(OD_t *od, uint8_t storageGroup);


/**
 * @defgroup CO_ODgetSetters Getters and setters
 * @{
 *
 * Getter and setter helpre functions for accessing different types of Object
 * Dictionary variables.
 */
/**
 * Get int8_t variable from Object Dictionary
 *
 * @param entry OD entry returned by @ref OD_find().
 * @param subIndex Sub-index of the variable from the OD object.
 * @param [out] val Value will be written there.
 *
 * @return Value from @ref ODR_t, "ODR_OK" in case of success.
 */
ODR_t OD_get_i8(const OD_entry_t *entry, uint16_t subIndex, int8_t *val);
/** Get int16_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_i16(const OD_entry_t *entry, uint16_t subIndex, int16_t *val);
/** Get int32_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_i32(const OD_entry_t *entry, uint16_t subIndex, int32_t *val);
/** Get int64_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_i64(const OD_entry_t *entry, uint16_t subIndex, int64_t *val);
/** Get uint8_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_u8(const OD_entry_t *entry, uint16_t subIndex, uint8_t *val);
/** Get uint16_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_u16(const OD_entry_t *entry, uint16_t subIndex, uint16_t *val);
/** Get uint32_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_u32(const OD_entry_t *entry, uint16_t subIndex, uint32_t *val);
/** Get uint64_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_u64(const OD_entry_t *entry, uint16_t subIndex, uint64_t *val);
/** Get float32_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_r32(const OD_entry_t *entry, uint16_t subIndex, float32_t *val);
/** Get float64_t variable from Object Dictionary, see @ref OD_get_i8 */
ODR_t OD_get_r64(const OD_entry_t *entry, uint16_t subIndex, float64_t *val);

/**
 * Set int8_t variable in Object Dictionary
 *
 * @param entry OD entry returned by @ref OD_find().
 * @param subIndex Sub-index of the variable from the OD object.
 * @param val Value to write.
 *
 * @return Value from @ref ODR_t, "ODR_OK" in case of success.
 */
ODR_t OD_set_i8(const OD_entry_t *entry, uint16_t subIndex, int8_t val);
/** Set int16_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_i16(const OD_entry_t *entry, uint16_t subIndex, int16_t val);
/** Set int16_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_i32(const OD_entry_t *entry, uint16_t subIndex, int32_t val);
/** Set int16_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_i64(const OD_entry_t *entry, uint16_t subIndex, int64_t val);
/** Set uint8_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_u8(const OD_entry_t *entry, uint16_t subIndex, uint8_t val);
/** Set uint16_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_u16(const OD_entry_t *entry, uint16_t subIndex, uint16_t val);
/** Set uint32_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_u32(const OD_entry_t *entry, uint16_t subIndex, uint32_t val);
/** Set uint64_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_u64(const OD_entry_t *entry, uint16_t subIndex, uint64_t val);
/** Set float32_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_r32(const OD_entry_t *entry, uint16_t subIndex, float32_t val);
/** Set float64_t variable in Object Dictionary, see @ref OD_set_i8 */
ODR_t OD_set_r64(const OD_entry_t *entry, uint16_t subIndex, float64_t val);
/** @} */ /* CO_ODgetSetters */


/**
 * @defgroup CO_ODdefinition OD definition objects
 * @{
 *
 * Types and functions used only for definition of Object Dictionary
 */
#if defined OD_DEFINITION || defined CO_DOXYGEN

/**
 * Types for OD object.
 */
typedef enum {
    /** This type corresponds to CANopen Object Dictionary object with object
     * code equal to VAR. OD object is type of @ref OD_obj_var_t and represents
     * single variable of any type (any length), located on sub-index 0. Other
     * sub-indexes are not used. */
    ODT_VAR = 0x01,
    /** This type corresponds to CANopen Object Dictionary object with object
     * code equal to ARRAY. OD object is type of @ref OD_obj_array_t and
     * represents array of variables with the same type, located on sub-indexes
     * above 0. Sub-index 0 is of type uint8_t and usually represents length of
     * the array. */
    ODT_ARR = 0x02,
    /** This type corresponds to CANopen Object Dictionary object with object
     * code equal to RECORD. This type of OD object represents structure of
     * the variables. Each variable from the structure can have own type and
     * own attribute. OD object is an array of elements of type
     * @ref OD_obj_var_t. Variable at sub-index 0 is of type uint8_t and usually
     * represents number of sub-elements in the structure. */
    ODT_REC = 0x03,
    /** ODT_VAR with additional low and high limit of the parameter value */
    ODT_VARL = 0x04,
    /** ODT_ARR with additional low and high limits of the parameter values */
    ODT_ARRL = 0x05,
    /** ODT_REC with additional low and high limits of the parameter values */
    ODT_RECL = 0x06,

    /** Same as ODT_VAR, but extended with OD_obj_extended_t type. It includes
     * additional pointer to IO extension and PDO flags */
    ODT_EVAR = 0x11,
    /** Same as ODT_ARR, but extended with OD_obj_extended_t type */
    ODT_EARR = 0x12,
    /** Same as ODT_REC, but extended with OD_obj_extended_t type */
    ODT_EREC = 0x13,
    /** Same as ODT_VARL, but extended with OD_obj_extended_t type */
    ODT_EVARL = 0x14,
    /** Same as ODT_ARRL, but extended with OD_obj_extended_t type. */
    ODT_EARRL = 0x15,
    /** Same as ODT_RECL, but extended with OD_obj_extended_t type. */
    ODT_ERECL = 0x16,

    /** Mask for basic type */
    ODT_TYPE_MASK = 0x0F,
    /** Mask for extension */
    ODT_EXTENSION_MASK = 0x10
} OD_objectTypes_t;

/**
 * Object for single OD variable, used for "VAR" and "RECORD" type OD objects
 */
typedef struct {
    void *data; /**< Pointer to data */
    OD_attr_t attribute; /**< Attribute bitfield, see @ref OD_attributes_t */
    OD_size_t dataLength; /**< Data length in bytes */
} OD_obj_var_t;

/**
 * Limits of the parameter value.
 */
typedef struct {
    int32_t low; /**< Low limit of the parameter value */
    int32_t high; /**< High limit of the parameter value */
} OD_limits_t;

/**
 * Object for single OD variable, used for "VAR" and "RECORD" type OD objects.
 * Additionally includes limits of the parameter value.
 */
typedef struct {
    void *data; /**< Pointer to data */
    OD_attr_t attribute; /**< Attribute bitfield, see @ref OD_attributes_t */
    OD_size_t dataLength; /**< Data length in bytes */
    OD_limits_t limit; /**< Limits of the parameter value */
} OD_obj_varLimits_t;

/**
 * Object for OD array of variables, used for "ARRAY" type OD objects
 */
typedef struct {
    uint8_t *data0; /**< Pointer to data for sub-index 0 */
    void *data; /**< Pointer to array of data */
    OD_attr_t attribute0; /**< Attribute bitfield for sub-index 0, see
                               @ref OD_attributes_t */
    OD_attr_t attribute; /**< Attribute bitfield for array elements */
    OD_size_t dataElementLength; /**< Data length of array elements in bytes */
    OD_size_t dataElementSizeof; /**< Sizeof one array element in bytes */
} OD_obj_array_t;

/**
 * Object for OD array of variables, used for "ARRAY" type OD objects.
 * Additionally includes limits of the parameter value for each array element
 * and separate attribute for each array element.
 */
typedef struct {
    uint8_t *data0; /**< Pointer to data for sub-index 0 */
    void *data; /**< Pointer to array of data */
    OD_limits_t *limits; /**< Pointer to array limits of the parameter value */
    OD_attr_t *attributes; /**< Pointer to array attributes */
    OD_attr_t attribute0; /**< Attribute bitfield for sub-index 0, see
                               @ref OD_attributes_t */
    OD_size_t dataElementLength; /**< Data length of array elements in bytes */
    OD_size_t dataElementSizeof; /**< Sizeof one array element in bytes */
} OD_obj_arrayLimAttr_t;

/**
 * Object pointed by @ref OD_obj_extended_t contains application specified
 * parameters for extended OD object
 */
typedef struct {
    /** Object on which read and write will operate */
    void *object;
    /** Application specified function pointer, see @ref OD_subEntry_t. */
    OD_size_t (*read)(OD_stream_t *stream, uint8_t subIndex,
                      void *buf, OD_size_t count, ODR_t *returnCode);
    /** Application specified function pointer, see @ref OD_subEntry_t. */
    OD_size_t (*write)(OD_stream_t *stream, uint8_t subIndex,
                       const void *buf, OD_size_t count, ODR_t *returnCode);
} OD_extensionIO_t;

/**
 * Object for extended type of OD variable, configurable by
 * @ref OD_extensionIO_init() function
 */
typedef struct {
    /** Pointer to PDO flags bit-field, see @ref OD_subEntry_t, may be NULL. */
    OD_flagsPDO_t *flagsPDO;
    /** Pointer to application specified IO extension, may be NULL. */
    OD_extensionIO_t *extIO;
    /** Pointer to original odObject, see @ref OD_entry_t. */
    const void *odObjectOriginal;
} OD_obj_extended_t;

#endif /* defined OD_DEFINITION */

/** @} */ /* CO_ODdefinition */


/** @} */

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* CO_OD_INTERFACE_H */
