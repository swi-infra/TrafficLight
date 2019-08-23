#include "legato.h"
#include "interfaces.h"

// Default polling timer to 1 minute
#ifndef AVC_POLLING_TIME_MINUTE
#  define AVC_POLLING_TIME_MINUTE 1
#endif

/* settings*/

// Name of the Config Tree you want to write to
#ifndef CONFIG_TREE_NAME
#  error "CONFIG_TREE_NAME undefined"
#endif

#define xstr(s) str(s)
#define str(s) #s

#define CONFIG_TREE_NAME_STR xstr(CONFIG_TREE_NAME)

// Paths in the config tree you want to write to
#define CONFIG_TREE_URL "/url"
#define CONFIG_TREE_INFO_EXITCODE_CHECKFLAG "/info/exitCode/checkFlag"
#define CONFIG_TREE_INFO_CONTENT_CHECKFLAG "/info/content/checkFlag"
#define CONFIG_TREE_POLLINGINTERVALSEC "/pollingIntervalSec"

//-------------------------------------------------------------------------------------------------
/**
 * AVC related variable
 */
//-------------------------------------------------------------------------------------------------

// Reference to AVC event handler
static le_avdata_SessionStateHandlerRef_t AvcEventHandlerRef = NULL;

// Reference to AVC Session handler
le_avdata_RequestSessionObjRef_t AvcSessionRef = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Struct defines each config entry's data type, path to config tree node and resource for comparison
 */
//--------------------------------------------------------------------------------------------------
typedef struct {
    le_avdata_DataType_t dataType;  ///< Type of data
    const char * configTreePathPtr; ///< Path to data in the configTree.
    void * resourcePathPtr;   ///< Path to the resource when accessed from AVC.
} ConfigEntry_t;

//--------------------------------------------------------------------------------------------------
/**
 * Holds information of the data type, path to where the config tree will be written, and contextPtr
 * tag.
 */
//--------------------------------------------------------------------------------------------------
const ConfigEntry_t ConfigEntries[] = {
    { LE_AVDATA_DATA_TYPE_STRING,
      CONFIG_TREE_URL,
      CONFIG_TREE_URL},
    { LE_AVDATA_DATA_TYPE_INT,
      CONFIG_TREE_POLLINGINTERVALSEC,
      CONFIG_TREE_POLLINGINTERVALSEC},
    { LE_AVDATA_DATA_TYPE_BOOL,
      CONFIG_TREE_INFO_EXITCODE_CHECKFLAG,
      CONFIG_TREE_INFO_EXITCODE_CHECKFLAG},
    { LE_AVDATA_DATA_TYPE_BOOL,
      CONFIG_TREE_INFO_CONTENT_CHECKFLAG,
      CONFIG_TREE_INFO_CONTENT_CHECKFLAG},
    /* ... insert other entries here ... */
};

//-------------------------------------------------------------------------------------------------
/**
 * Functions to write different data types (bool, string, int, float) to the config tree given the
 * path and the data
 */
//-------------------------------------------------------------------------------------------------
static void WriteBoolToConfig
(
    const char * pathToDataPtr,
    bool dataToWrite
)
{
    le_cfg_IteratorRef_t iteratorRef;

    iteratorRef = le_cfg_CreateWriteTxn(CONFIG_TREE_NAME_STR ":");
    le_cfg_SetBool(iteratorRef, pathToDataPtr, dataToWrite);
    le_cfg_CommitTxn(iteratorRef);
}

static void WriteStringToConfig
(
    const char * pathToDataPtr,
    char * dataToWrite
)
{
    le_cfg_IteratorRef_t iteratorRef;

    iteratorRef = le_cfg_CreateWriteTxn(CONFIG_TREE_NAME_STR ":");
    le_cfg_SetString(iteratorRef, pathToDataPtr, dataToWrite);
    le_cfg_CommitTxn(iteratorRef);
}

static void WriteIntToConfig
(
    const char * pathToDataPtr,
    int dataToWrite
)
{
    le_cfg_IteratorRef_t iteratorRef;

    iteratorRef = le_cfg_CreateWriteTxn(CONFIG_TREE_NAME_STR ":");
    le_cfg_SetInt(iteratorRef, pathToDataPtr, dataToWrite);
    le_cfg_CommitTxn(iteratorRef);
}

static void WriteFloatToConfig
(
    const char * pathToDataPtr,
    double dataToWrite
)
{
    le_cfg_IteratorRef_t iteratorRef;

    iteratorRef = le_cfg_CreateWriteTxn(CONFIG_TREE_NAME_STR ":");
    le_cfg_SetFloat(iteratorRef, pathToDataPtr, dataToWrite);
    le_cfg_CommitTxn(iteratorRef);
}

//-------------------------------------------------------------------------------------------------
/**
 * This function is returned whenever a write operation occurs from AirVantage.
 * It will write the data sent from the resource to the path specified.
 */
//-------------------------------------------------------------------------------------------------
static void ConfigSettingHandler
(
    const char* path,
    le_avdata_AccessType_t accessType,
    le_avdata_ArgumentListRef_t argumentList,
    void* contextPtr
)
{
    const char * pathToDataPtr = NULL;
    le_avdata_DataType_t dataType = LE_AVDATA_DATA_TYPE_NONE;

    // Find the pathToDataPtr and the dataType through the contextPtr sent by the handler.
    int i;
    for (i = 0; i < NUM_ARRAY_MEMBERS(ConfigEntries); i++)
    {
        if (contextPtr == ConfigEntries[i].resourcePathPtr)
        {
            pathToDataPtr = ConfigEntries[i].configTreePathPtr;
            dataType = ConfigEntries[i].dataType;
            break;
        }
    }

    if (pathToDataPtr == NULL)
    {
        LE_FATAL("Unable to find data for context %p", contextPtr);
    }

    LE_INFO("------------- Server writes to: %s%s -------------", CONFIG_TREE_NAME_STR ":", (char *) contextPtr);
    LE_INFO("path to data is : %s", pathToDataPtr);

    // Get the data with their respective data types
    le_result_t resultGet = LE_FAULT;
    switch(dataType)
    {
        case LE_AVDATA_DATA_TYPE_STRING:
            {
                char bufferString[LE_CFG_STR_LEN_BYTES] = {0};
                resultGet = le_avdata_GetString(pathToDataPtr, bufferString, sizeof(bufferString));
                if (resultGet == LE_OK)
                {
                    LE_INFO("String being written is: '%s'", bufferString);
                    WriteStringToConfig(pathToDataPtr, bufferString);
                }
            }
            break;

        case LE_AVDATA_DATA_TYPE_INT:
            {
                int32_t bufferInt = 0;
                resultGet = le_avdata_GetInt(pathToDataPtr, &bufferInt);
                if (resultGet == LE_OK)
                {
                    LE_INFO("Int being written is: %i", bufferInt);
                    WriteIntToConfig(pathToDataPtr, bufferInt);
                }
            }
            break;

        case LE_AVDATA_DATA_TYPE_FLOAT:
            {
                double bufferFloat = 0;
                resultGet = le_avdata_GetFloat(pathToDataPtr, &bufferFloat);
                if (resultGet == LE_OK)
                {
                    LE_INFO("Float being written is: %f", bufferFloat);
                    WriteFloatToConfig(pathToDataPtr, bufferFloat);
                }
            }
            break;

        case LE_AVDATA_DATA_TYPE_BOOL:
            {
                bool bufferBool = false;
                resultGet = le_avdata_GetBool(pathToDataPtr, &bufferBool);
                if (resultGet == LE_OK)
                {
                    LE_INFO("Bool being written is: %s", bufferBool ? "true" : "false");
                    WriteBoolToConfig(pathToDataPtr, bufferBool);
                }
            }
            break;

        case LE_AVDATA_DATA_TYPE_NONE:
            LE_ERROR("Error: dataType was not assigned (type none), pathToDataPtr may be corrupt");
            break;

        default:
            LE_ERROR("Asset Data type not handled: %d", dataType);
            break;
    }

    if(resultGet != LE_OK)
    {
        LE_ERROR("Unable to retreive asset data at '%s': %d", pathToDataPtr, resultGet);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Function relevant to AirVantage server connection
 */
//-------------------------------------------------------------------------------------------------
static void AppTerminationHandler
(
    int sigNum
)
{
    LE_INFO("Close AVC session");

    if (AvcSessionRef != NULL)
    {
        le_avdata_ReleaseSession(AvcSessionRef);
    }

    if (AvcEventHandlerRef != NULL)
    {
        // Unregister
        LE_INFO("Unregister the session handler");
        le_avdata_RemoveSessionStateHandler(AvcEventHandlerRef);
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Status handler for avcService updates
 */
//-------------------------------------------------------------------------------------------------
static void AvcStatusHandler
(
    le_avdata_SessionState_t updateStatus,
    void* contextPtr
)
{
    switch (updateStatus)
    {
        case LE_AVDATA_SESSION_STARTED:
            LE_INFO("Legato session started successfully");
            break;
        case LE_AVDATA_SESSION_STOPPED:
            LE_INFO("Legato session stopped");
            break;
        default:
            LE_ERROR("Error: AvcStatusHandler called with no updateStatus passed to the function");
    }
}

COMPONENT_INIT
{
    le_result_t resultCreateResources;
    int i = 0;

    LE_INFO("Start Legato writeConfigTree App");

    // Add app termination sequence
    le_sig_Block(SIGTERM);
    le_sig_SetEventHandler(SIGTERM, AppTerminationHandler);

#if AVC_POLLING_TIME_MINUTE != 0
    // Set a polling timer as to make the target automatically connect to
    // Airvantage every few minutes.
    le_result_t setPollingResult = le_avc_SetPollingTimer(AVC_POLLING_TIME_MINUTE);
    if (setPollingResult == LE_OUT_OF_RANGE)
    {
        LE_FATAL("Polling timer is set out of range (%d).", AVC_POLLING_TIME_MINUTE);
    }
#else
    LE_INFO("Polling timer not set");
#endif

    // Start AVC Session

    // Register AVC handler
    AvcEventHandlerRef = le_avdata_AddSessionStateHandler(AvcStatusHandler, NULL);

    // Request AVC session. Note: AVC handler must be registered prior starting a session
    AvcSessionRef = le_avdata_RequestSession();
    if (NULL == AvcSessionRef)
    {
        LE_ERROR("AirVantage Connection Controller does not start.");
    }
    else
    {
        LE_INFO("AirVantage Connection Controller started.");
    }

    LE_INFO("Started LWM2M session with AirVantage");

    // Create resources
    LE_INFO("Create instances AssetData");

    for (i = 0; i < NUM_ARRAY_MEMBERS(ConfigEntries); i++)
    {
        LE_INFO("Registering resource '%s' ...", ConfigEntries[i].configTreePathPtr);

        resultCreateResources = le_avdata_CreateResource(ConfigEntries[i].configTreePathPtr,
                                                         LE_AVDATA_ACCESS_SETTING);
        if (LE_FAULT == resultCreateResources)
        {
            LE_ERROR("Error in creating %s", ConfigEntries[i].configTreePathPtr);
        }

        le_avdata_AddResourceEventHandler(ConfigEntries[i].configTreePathPtr,
                                          ConfigSettingHandler,
                                          ConfigEntries[i].resourcePathPtr);
    }
}
