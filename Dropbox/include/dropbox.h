/*!
 * \file    dropbox.h
 * \brief   Unofficial C dropbox API.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#ifndef DROPBOX_H
#define DROPBOX_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
#include <stdbool.h>
    
#define DRBVAL_SIZE_XSMALL "xs"
#define DRBVAL_SIZE_SMALL  "s"
#define DRBVAL_SIZE_MEDIUM "m"
#define DRBVAL_SIZE_LARGE  "l"
#define DRBVAL_SIZE_XLARGE "xl"
#define DRBVAL_ROOT_DROPBOX "dropbox"
#define DRBVAL_ROOT_SANDBOX "sandbox"
#define DRBVAL_ROOT_AUTO    "auto"
#define DRBVAL_IGNORE_BOOL -1
#define DRBVAL_IGNORE_STR  NULL
#define DRBVAL_IGNORE_INT  -1
#define DRBVAL_IGNORE_SIZE DRBVAL_IGNORE_STR
#define DRBVAL_IGNORE_PTR  NULL

/*!
 * \struct  drbClient
 * \breif   Dropbox client.
 *
 * Must be freed with drbDestroyClient.
 */
typedef struct drbClient drbClient;

/*!
 * \struct  drbOAuthToken
 * \breif   OAuth 1.0 Token (credentials).
 */
typedef struct {
    char* key;
    char* secret;
} drbOAuthToken;


/*!
 * \struct  drbAccountInfo
 * \breif   Dropbox account informations.
 *
 * Missing fields in the server answer are left blank with NULL value.
 * Check https://www.dropbox.com/developers/core/docs#account-info for more
 * details about these fields.
 *
 * Must be freed with drbDestroyAccountInfo.
 */
typedef struct {
    char* referralLink;
    char* displayName;
    unsigned int* uid;
    char* country;
    struct {
        unsigned int* datastores;
        unsigned int* shared;
        unsigned int* quota;
        unsigned int* normal;
    } quotaInfo ;
    char* email;
} drbAccountInfo;

/*!
 * \struct  drbMetadataList
 * \breif   Dropbox file or folder metadata.
 *
 * Missing fields in the server answer are left blank with NULL value.
 * Check https://www.dropbox.com/developers/core/docs#metadata for more details
 * about these fields.
 *
 * Must be freed with drbDestroyMetadata.
 */
typedef struct drbMetadataList drbMetadataList;

typedef struct {
    unsigned int*  bytes;
    char* clientMtime;
    char* icon;
    bool* isDir;
    char* mimeType;
    char* modified;
    char* path;
    char* rev;
    unsigned int* revision;
    char* root;
    char* size;
    bool* thumbExists;
    bool* isDeleted;
    char* hash; /*!< Only defined for folders (isDir = true). */
    drbMetadataList* contents;
} drbMetadata;

struct drbMetadataList{
    drbMetadata **array; /*!< List of all metadata. */
    size_t size;  /*!< List size. */
};

/*!
 * \struct  drbLink
 * \breif   Dropbox temporary link to a file.
 *
 * Missing fields in the server answer are left blank with NULL value.
 * Check https://www.dropbox.com/developers/core/docs#shares for more details
 * about these fields.
 *
 * Must be freed with drbDestroyLink.
 */
typedef struct {
    char* url;
    char* expires;
} drbLink;

/*!
 * \struct  drbLink
 * \breif   Dropbox file copy reference.
 *
 * Missing fields in the server answer are left blank with NULL value.
 * Check https://www.dropbox.com/developers/core/docs#copy_ref for more details
 * about these fields.
 *
 * Must be freed with drbDestroyCopyRef.
 */
typedef struct {
    char* copyRef;
    char* expires;
} drbCopyRef;

/*!
 * \struct  drbDeltaEntry
 * \breif   Dropbox delta entry.
 *
 * Missing fields in the server answer are left blank with NULL value.
 * Check https://www.dropbox.com/developers/core/docs#delta for more details
 * about these fields.
 */
typedef struct {
    char* path;
    drbMetadata* metadata;
} drbDeltaEntry;

/*!
 * \struct  drbDelta
 * \breif   Dropbox delta informations.
 *
 * Missing fields in the server answer are left blank with NULL value.
 * Check https://www.dropbox.com/developers/core/docs#delta for more details
 * about these fields.
 *
 * Must be freed with drbDestroyDelta.
 */
typedef struct {
    bool* reset;
    char* cursor;
    bool* hasMore;
    struct {
        drbDeltaEntry* array; /*!< List of all delta entries. */
        size_t size;          /*!< List size. */
    } entries;
} drbDelta;

/*!
 * \struct  drbDelta
 * \breif   Dropbox delta informations.
 *
 * Missing fields in the server answer are left blank with NULL value.
 * Check https://www.dropbox.com/developers/core/docs#longpoll-delta for
 * more details about these fields.
 *
 * Must be freed with drbDestroyLongPollDelta.
 */
typedef struct {
    bool* changes;
    unsigned int* backoff;
} drbPollDelta;

    
/*!
 * \breif Function options and expected arguement type.
 *
 * Depending on the function, an option is either:
 *   1. illegal
 *   2. optional
 *   3. required by the dropbox server
 *   4. required by the function itself
 *
 * If this last kind of options are forgotten, the DRBERR_MISSING_OPT error code
 * is returned by the function. Otherwise, for illegal options or the
 * forgetting of an option of the third kind, an http error is returned.
 *
 */
enum {
    DRBOPT_CURSOR,          /*!< string  */
    DRBOPT_FILE_LIMIT,      /*!< integer */
    DRBOPT_FORMAT,          /*!< string  */
    DRBOPT_FROM_COPY_REF,   /*!< string  */
    DRBOPT_FROM_PATH,       /*!< string  */
    DRBOPT_HASH,            /*!< string  */
    DRBOPT_INCL_DELETED,    /*!< boolean */
    DRBOPT_LIST,            /*!< boolean */
    DRBOPT_LOCALE,          /*!< string  */
    DRBOPT_OVERWRITE,       /*!< boolean */
    DRBOPT_PATH,            /*!< string  */
    DRBOPT_PARENT_REV,      /*!< string  */
    DRBOPT_QUERY,           /*!< string  */
    DRBOPT_REV,             /*!< string  */
    DRBOPT_REV_LIMIT,       /*!< integer */
    DRBOPT_ROOT,            /*!< string  */
    DRBOPT_SHORT_URL,       /*!< boolean */
    DRBOPT_SIZE,            /*!< string  */
    DRBOPT_TO_PATH,         /*!< string  */
    DRBOPT_IO_DATA,         /*!< void* (e.i FILE*) */
    DRBOPT_IO_FUNC,         /*!< size_t(*)(const void*,size_t,size_t,void*) */
    DRBOPT_TIMEOUT,         /*!< integer */
    DRBOPT_NETWORK_TIMEOUT, /*!< integer */
    DRBOPT_INCL_MEDIA_INFO, /*!< boolean */
    DRBOPT_PATH_PREFIX,     /*!< string  */
    
    DRBOPT_END,             /*!< no argument! ALWAYS REQUIRED as last option */
};

/*!
 * Error code returned by functions. However, dropbox http errors codes (>100)
 * are not defined here. Check https://www.dropbox.com/developers/core/docs
 * under "Standard API errors" for more details about these kind of errors.
 */
enum {
    DRBERR_OK = 0,         /*!< 0: no error */
    DRBERR_MISSING_OPT,    /*!< 1: an option required by the function is missing */
    DRBERR_UNKNOWN_OPT,    /*!< 2: unknown option code encountered */
    DRBERR_DUPLICATED_OPT, /*!< 3: an option code was set twice or more */
    DRBERR_INVALID_VAL,    /*!< 4: invalid argument value encountered */
    DRBERR_MALLOC,         /*!< 5: memory allocation failed */
    DRBERR_UNKNOWN,        /*!< 6: something that shouldn't happen, has happened */
    DRBERR_NETWORK,        /*!< 7: network issue */
    DRBERR_TIMEOUT,        /*!< 8: request timed out */
};

/*!
 * \brief   Sets up the programm environment that dropbox library needs.
 * \return  void
 */
void drbInit();

/*!
 * \brief   Release ressources acquired by drbInit.
 * \return  void
 */
void drbCleanup();

/*!
 * \brief   Create and initilize a drbClient.
 * \param   cKey      consumer key (Client credentials)
 * \param   cSecret   consumer secret (Client credentials)
 * \param   tKey      request or access key (Temporary or Token credentials)
 * \param   tSecret   request or access secret (Temporary or Token credentials)
 * \return  drbClient pointer must be freed with drbDestroyClient by caller
 */
drbClient* drbCreateClient(const char* cKey, const char* cSecret, const char* tKey, const char* tSecret);

/*!
 * \brief  Obtain the request token (temporary credentials).
 *
 * Achieves the 1st step of OAuth 1.0 protocol by obtaining the temporary creds.
 * The token and its fields lifetime is only guaranteed until the next API
 * function call. If the user code need them afterward, they must be duplicated.
 *
 * \param   cli   client to authorize dropbox access.
 * \return  obtained request token (must NOT be freed or modified by caller).
 */
drbOAuthToken* drbObtainRequestToken(drbClient* cli);

/*!
 * \brief   Build the url for client accces authorization.
 *
 * Build url for 2nd step of OAuth 1.0 protocol (Resource Owner Authorization).
 *
 * \param   cli   client to authorize dropbox access.
 * \return  url to authorize client to acces the user dropbox. Must be freed!
 */
char* drbBuildAuthorizeUrl(drbOAuthToken* reqTok);

/*!
 * \brief   Obtain the access token (Token credentials).
 *
 * Achieves the last step of OAuth 1.0 protocol by obtaining the token creds.
 * The token and its fields lifetime is only guaranteed until the next API
 * function call. If the user code need them afterward, they must be duplicated.
 *
 * \param   cli   client to authorize dropbox access.
 * \return  obtained access token (must NOT be freed or modified by caller)
 */
drbOAuthToken* drbObtainAccessToken(drbClient* cli);
    
/*!
 * \brief   Set the default options agruments.
 *
 * Options could be removed with DRBVAL_IGNORE_XXX constants (exept for the
 * DRBOPT_NETWORK_TIMEOUT valeu, which could only be changed).
 *
 * \param   cli   authenticated dropbox client
 * \param   ...   default option/value pairs to set.
 * \return  void
 */
int drbSetDefault(drbClient* cli, ...);
    
/*!
 * \brief   Get account general informations.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   account informations (drbAccountInfo*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbGetAccountInfo(drbClient* cli, void** output, ...);

/*!
 * \brief   Get a file or folder metadata.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   item metadata (drbMetadata*) or error (char*)
 * \param       ...      legal option/value pairs:
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_FILE_LIMIT
 *                         -# DRBOPT_HASH
 *                         -# DRBOPT_LIST
 *                         -# DRBOPT_INCL_DELETED
 *                         -# DRBOPT_REV
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
//int drbGetMetadata(drbClient* cli, drbMetadata** meta, ...);
int drbGetMetadata(drbClient* cli, void** output, ...);

/*!
 * \brief   Download a file.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   file metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_IO_DATA, e.i. FILE*  (required)
 *                         -# DRBOPT_IO_FUNC, e.i. fwrite (required)
 *                         -# DRBOPT_REV
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbGetFile(drbClient* cli, void** output, ...);

/*!
 * \brief   Get a file revisions.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   file revisions list (drbMetadataList*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_REV_LIMIT
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbGetRevisions(drbClient* cli, void** output, ...);

/*!
 * \brief   Search a file or a folder.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   founded items list (drbMetadataList*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_QUERY
 *                         -# DRBOPT_FILE_LIMIT
 *                         -# DRBOPT_INCL_DELETED
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbSearch(drbClient* cli, void** output, ...);

/*!
 * \brief   Download a thumbnail for an image file.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   image file metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_IO_DATA, e.i. FILE*  (required)
 *                         -# DRBOPT_IO_FUNC, e.i. fwrite (required)
 *                         -# DRBOPT_FORMAT
 *                         -# DRBOPT_SIZE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbGetThumbnail(drbClient* cli, void** output, ...);

/*!
 * \brief   Copy a file or folder.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   new copied item metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_FROM_PATH (required)
 *                         -# DRBOPT_TO_PATH or DRBOPT_FROM_COPY_REF (required)
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbCopy(drbClient* cli, void** output, ...);

/*!
 * \brief   Create a folder.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   created folder metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbCreateFolder(drbClient* cli, void** output, ...);

/*!
 * \brief   Delete a file or folder.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   deleted item metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbDelete(drbClient* cli, void** output, ...);

/*!
 * \brief   Move a file or folder.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   moved item metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_FROM_PATH (required)
 *                         -# DRBOPT_TO_PATH or DRBOPT_FROM_COPY_REF (required)
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbMove(drbClient* cli, void** ouput, ...);

/*!
 * \brief   Get changed files and folders.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   delta informations (drbDelta*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_CURSOR
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbGetDelta(drbClient* cli, void** output, ...);

/*!
 * \brief   Restore a file.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   restored file metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_REV  (required)
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbRestore(drbClient* cli, void** output, ...);

/*!
 * \brief   Create a dropbox link to a file.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   link to the file (drbLink*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_SHORT_URL
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbShare(drbClient* cli, void** output, ...);

/*!
 * \brief   Create a direct link to a file.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   link to the file (drbLink*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbGetMedia(drbClient* cli, void** output, ...);

/*!
 * \brief   Create a copy reference to a file.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   copied file reference (drbCopyRef*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbGetCopyRef(drbClient* cli, void** output, ...);

/*!
 * \brief   Upload a file.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   uploaded file metadata (drbMetadata*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_ROOT (required)
 *                         -# DRBOPT_PATH (required)
 *                         -# DRBOPT_IO_DATA (required)
 *                         -# DRBOPT_IO_FUNC (required)
 *                         -# DRBOPT_OVERWRITE
 *                         -# DRBOPT_PARENT_REV
 *                         -# DRBOPT_LOCALE
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbPutFile(drbClient* cli, void** output, ...);

/*!
 * \brief   Wait for changes on the account.
 * \param       cli      authenticated dropbox client
 * \param[out]  output   poll result (drbPollDelta*) or error (char*)
 * \param       ...      option/value pairs (function arguments) :
 *                         -# DRBOPT_CURSOR (required)
 *                         -# DRBOPT_TIMEOUT (required)
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbLongPollDelta(drbClient* cli, void** output, ...);
    
    
void drbDestroyClient(drbClient* cli);
void drbDestroyCopyRef(drbCopyRef* ref);
void drbDestroyMedia(drbLink* link);
void drbDestroyMetadata(drbMetadata* meta, bool destroyList);
void drbDestroyMetadataList(drbMetadataList* list, bool destroyMetadata);
void drbDestroyAccountInfo(drbAccountInfo* info);
void drbDestroyLink(drbLink* link);
void drbDestroyDelta(drbDelta* delta, bool withMetadata);
void drbDestroyPollDelta(drbPollDelta* poll);
    
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* DROPBOX_H */
