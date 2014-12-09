/*!
 * \file    dropbox.c
 * \brief   Unofficial C dropbox API.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdarg.h>
#include <memStream.h>
#include "dropbox.h"
#include "dropboxOAuth.h"
#include "dropboxJson.h"
#include "dropboxUtils.h"


enum {
    DRBBIT_VOID,
    DRBBIT_CURSOR          = 1<<DRBOPT_CURSOR,
    DRBBIT_FILE_LIMIT      = 1<<DRBOPT_FILE_LIMIT,
    DRBBIT_FORMAT          = 1<<DRBOPT_FORMAT,
    DRBBIT_FROM_COPY_REF   = 1<<DRBOPT_FROM_COPY_REF,
    DRBBIT_FROM_PATH       = 1<<DRBOPT_FROM_PATH,
    DRBBIT_HASH            = 1<<DRBOPT_HASH,
    DRBBIT_INCL_DELETED    = 1<<DRBOPT_INCL_DELETED,
    DRBBIT_LIST            = 1<<DRBOPT_LIST,
    DRBBIT_LOCALE          = 1<<DRBOPT_LOCALE,
    DRBBIT_OVERWRITE       = 1<<DRBOPT_OVERWRITE,
    DRBBIT_PATH            = 1<<DRBOPT_PATH,
    DRBBIT_PARENT_REV      = 1<<DRBOPT_PARENT_REV,
    DRBBIT_QUERY           = 1<<DRBOPT_QUERY,
    DRBBIT_REV             = 1<<DRBOPT_REV,
    DRBBIT_REV_LIMIT       = 1<<DRBOPT_REV_LIMIT,
    DRBBIT_ROOT            = 1<<DRBOPT_ROOT,
    DRBBIT_SHORT_URL       = 1<<DRBOPT_SHORT_URL,
    DRBBIT_SIZE            = 1<<DRBOPT_SIZE,
    DRBBIT_TO_PATH         = 1<<DRBOPT_TO_PATH,
    DRBBIT_IO_DATA         = 1<<DRBOPT_IO_DATA,
    DRBBIT_IO_FUNC         = 1<<DRBOPT_IO_FUNC,
    DRBBIT_TIMEOUT         = 1<<DRBOPT_TIMEOUT,
    DRBBIT_NETWORK_TIMEOUT = 1<<DRBOPT_NETWORK_TIMEOUT,
    DRBBIT_INCL_MEDIA_INFO = 1<<DRBOPT_INCL_MEDIA_INFO,
    DRBBIT_PATH_PREFIX     = 1<<DRBOPT_PATH_PREFIX,
    
    DRBBIT_END             = 1<<DRBOPT_END,
};

// Special Arguments
static const long DRBSA_ACC_INFO       = DRBBIT_NETWORK_TIMEOUT;
static const long DRBSA_GET_FILES      = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH| DRBBIT_IO_DATA | DRBBIT_IO_FUNC;
static const long DRBSA_PUT_FILES      = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH | DRBBIT_IO_DATA | DRBBIT_IO_FUNC;
static const long DRBSA_METADATA       = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBSA_DELTA          = DRBBIT_NETWORK_TIMEOUT;
static const long DRBSA_REVISIONS      = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBSA_RESTORE        = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBSA_SEARCH         = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBSA_THUMBNAILS     = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH | DRBBIT_IO_DATA | DRBBIT_IO_FUNC;
static const long DRBSA_SHARES         = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBSA_MEDIA          = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBSA_COPY           = DRBBIT_NETWORK_TIMEOUT;
static const long DRBSA_COPY_REF       = DRBBIT_NETWORK_TIMEOUT | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBSA_CREATE_FOLDER  = DRBBIT_NETWORK_TIMEOUT;
static const long DRBSA_DELETE         = DRBBIT_NETWORK_TIMEOUT;
static const long DRBSA_MOVE           = DRBBIT_NETWORK_TIMEOUT;
static const long DRBSA_LONGPOLL_DELTA = DRBBIT_VOID;

// Regular Arguments
static const long DRBRA_ACC_INFO       = DRBBIT_LOCALE;
static const long DRBRA_GET_FILES      = DRBBIT_REV ;
static const long DRBRA_PUT_FILES      = DRBBIT_LOCALE | DRBBIT_OVERWRITE | DRBBIT_PARENT_REV;
static const long DRBRA_METADATA       = DRBBIT_LOCALE | DRBBIT_FILE_LIMIT | DRBBIT_HASH | DRBBIT_LIST | DRBBIT_INCL_DELETED | DRBBIT_REV | DRBBIT_INCL_MEDIA_INFO;
static const long DRBRA_DELTA          = DRBBIT_LOCALE | DRBBIT_CURSOR | DRBBIT_PATH_PREFIX | DRBBIT_INCL_MEDIA_INFO;
static const long DRBRA_REVISIONS      = DRBBIT_LOCALE | DRBBIT_REV_LIMIT;
static const long DRBRA_RESTORE        = DRBBIT_LOCALE | DRBBIT_REV;
static const long DRBRA_SEARCH         = DRBBIT_LOCALE | DRBBIT_QUERY | DRBBIT_FILE_LIMIT | DRBBIT_INCL_DELETED;
static const long DRBRA_THUMBNAILS     = DRBBIT_FORMAT | DRBBIT_SIZE;
static const long DRBRA_SHARES         = DRBBIT_LOCALE | DRBBIT_SHORT_URL;
static const long DRBRA_MEDIA          = DRBBIT_LOCALE;
static const long DRBRA_COPY           = DRBBIT_LOCALE | DRBBIT_ROOT | DRBBIT_FROM_PATH | DRBBIT_TO_PATH | DRBBIT_FROM_COPY_REF;
static const long DRBRA_COPY_REF       = DRBBIT_VOID;
static const long DRBRA_CREATE_FOLDER  = DRBBIT_LOCALE | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBRA_DELETE         = DRBBIT_LOCALE | DRBBIT_ROOT | DRBBIT_PATH;
static const long DRBRA_MOVE           = DRBBIT_LOCALE | DRBBIT_ROOT | DRBBIT_FROM_PATH | DRBBIT_TO_PATH;
static const long DRBRA_LONGPOLL_DELTA = DRBBIT_CURSOR | DRBBIT_TIMEOUT;

// Dropbox API URIs
static const char* DRBURI_REQUEST        = "https://api.dropbox.com/1/oauth/request_token";
static const char* DRBURI_AUTHORIZATION  = "https://www.dropbox.com/1/oauth/authorize";
static const char* DRBURI_ACCESS         = "https://api.dropbox.com/1/oauth/access_token";
static const char* DRBURI_ACCOUNT_INFO   = "https://api.dropbox.com/1/account/info";
static const char* DRBURI_METADATA       = "https://api.dropbox.com/1/metadata";
static const char* DRBURI_GET_FILES      = "https://api-content.dropbox.com/1/files";
static const char* DRBURI_PUT_FILES      = "https://api-content.dropbox.com/1/files_put";
static const char* DRBURI_REVISIONS      = "https://api.dropbox.com/1/revisions";
static const char* DRBURI_SEARCH         = "https://api.dropbox.com/1/search";
static const char* DRBURI_THUMBNAILS     = "https://api-content.dropbox.com/1/thumbnails";
static const char* DRBURI_COPY           = "https://api.dropbox.com/1/fileops/copy";
static const char* DRBURI_CREATE_FOLDER  = "https://api.dropbox.com/1/fileops/create_folder";
static const char* DRBURI_DELETE         = "https://api.dropbox.com/1/fileops/delete";
static const char* DRBURI_MOVE           = "https://api.dropbox.com/1/fileops/move";
static const char* DRBURI_DELTA          = "https://api.dropbox.com/1/delta";
static const char* DRBURI_RESTORE        = "https://api.dropbox.com/1/restore";
static const char* DRBURI_SHARES         = "https://api.dropbox.com/1/shares";
static const char* DRBURI_MEDIA          = "https://api.dropbox.com/1/media";
static const char* DRBURI_COPY_REF       = "https://api.dropbox.com/1/copy_ref";
static const char* DRBURI_LONGPOLL_DELTA = "https://api-notify.dropbox.com/1/longpoll_delta";

// Special Handler arguments array Indexs
enum {DRBSHI_NETWORK_TIMEOUT, DRBSHI_ROOT, DRBSHI_PATH, DRBSHI_IO_DATA, DRBSHI_IO_FUNC};


/*!
 * \struct  drbOAuthToken
 * \breif   Option expected argument types''.
 */
typedef enum {
    DRBTYPE_BOOL,
    DRBTYPE_STR,
    DRBTYPE_INT,
    DRBTYPE_PATH,
    DRBTYPE_PTR,
    DRBTYPE_VAL,
} drbOptType;

/*!
 * \brief   Convert the current option argument to a string.
 * \param       ap        va_list to parse
 * \param       type      expected option argument type
 * \param[out]  value     converted argument value
 * \param[out]  ignored   indicates whether the option is ignored or not
 * \return  Error code (DRBERR_XXX).
 */
static int drbGetOptArg(va_list* ap, drbOptType type, drbOptArg* arg, bool* ignored) {
    int err = DRBERR_OK;
    *ignored = false;
    
    switch (type) {
        case DRBTYPE_BOOL: {
            int boolValue = va_arg(*ap, int);
            if (boolValue != DRBVAL_IGNORE_BOOL) {
                if((arg->str = strdup(boolValue ? "true" : "false")) == NULL )
                    err = DRBERR_MALLOC; // strdup went wrong
            } else *ignored = true;
            break;
        }
        case DRBTYPE_INT: {
            int intValue = va_arg(*ap, int);
            if (intValue != DRBVAL_IGNORE_INT) {
                if (intValue >= 0) {
                    if(asprintf(&arg->str, "%d", intValue) == -1)
                        err = DRBERR_MALLOC; // asprintf went wrong
                } else err = DRBERR_INVALID_VAL;
            } else *ignored = true;
            break;
        }
        case DRBTYPE_STR: {
            char *strValue = va_arg(*ap, char*);
            if (strValue != DRBVAL_IGNORE_STR) {
                if((arg->str = drbStrDup(strValue)) == NULL)
                    err = DRBERR_MALLOC;  // strdup went wrong
            } else *ignored = true;
            break;
        }
        case DRBTYPE_PATH: {
            char *strValue = va_arg(*ap, char*);
            if (strValue != DRBVAL_IGNORE_STR) {
                if ((arg->str = drbEncodePath(strValue)) == NULL) {
                    err = DRBERR_MALLOC;
                }
            } else *ignored = true;
            break;
        }
        case DRBTYPE_PTR: {
            void *ptrValue = va_arg(*ap, void*);
            if (ptrValue != DRBVAL_IGNORE_PTR) {
                arg->ptr = ptrValue;
            } else *ignored = true;
            break;
        }
        case DRBTYPE_VAL: {
            int intValue = va_arg(*ap, int);
            if (intValue != DRBVAL_IGNORE_INT) {
                arg->value = intValue;
            } else *ignored = true;
            break;
        }
        default: {
            err = DRBERR_UNKNOWN;
            break;
        }
    }
    
    return err;
}

/*!
 * \brief   Get the option name and type.
 * \param       opt    option to identify
 * \param[out]  name   option name
 * \param[out]  type   option type
 * \return  indicates whether the option was identified or not.
 */
static bool drbGetOptAttr(int opt, char** name, drbOptType* type) {
    switch (opt) {
        case DRBOPT_CURSOR:          *name = "cursor",          *type = DRBTYPE_STR;  break;
        case DRBOPT_FILE_LIMIT:      *name = "file_limit",      *type = DRBTYPE_INT;  break;
        case DRBOPT_FORMAT:          *name = "format",          *type = DRBTYPE_STR;  break;
        case DRBOPT_FROM_COPY_REF:   *name = "from_copy_ref",   *type = DRBTYPE_STR;  break;
        case DRBOPT_FROM_PATH:       *name = "from_path",       *type = DRBTYPE_PATH; break;
        case DRBOPT_HASH:            *name = "hash",            *type = DRBTYPE_STR;  break;
        case DRBOPT_INCL_DELETED:    *name = "include_deleted", *type = DRBTYPE_BOOL; break;
        case DRBOPT_LIST:            *name = "list",            *type = DRBTYPE_BOOL; break;
        case DRBOPT_LOCALE:          *name = "locale",          *type = DRBTYPE_STR;  break;
        case DRBOPT_OVERWRITE:       *name = "overwrite",       *type = DRBTYPE_BOOL; break;
        case DRBOPT_PATH:            *name = "path",            *type = DRBTYPE_PATH; break;
        case DRBOPT_PARENT_REV:      *name = "parent_rev",      *type = DRBTYPE_STR;  break;
        case DRBOPT_QUERY:           *name = "query",           *type = DRBTYPE_STR;  break;
        case DRBOPT_REV:             *name = "rev",             *type = DRBTYPE_STR;  break;
        case DRBOPT_REV_LIMIT:       *name = "rev_limit",       *type = DRBTYPE_INT;  break;
        case DRBOPT_ROOT:            *name = "root",            *type = DRBTYPE_STR;  break;
        case DRBOPT_SHORT_URL:       *name = "short_url",       *type = DRBTYPE_BOOL; break;
        case DRBOPT_SIZE:            *name = "size",            *type = DRBTYPE_STR;  break;
        case DRBOPT_TO_PATH:         *name = "to_path",         *type = DRBTYPE_PATH; break;
        case DRBOPT_IO_DATA:         *name = NULL,              *type = DRBTYPE_PTR;  break;
        case DRBOPT_IO_FUNC:         *name = NULL,              *type = DRBTYPE_PTR;  break;
        case DRBOPT_TIMEOUT:         *name = "timeout",         *type = DRBTYPE_INT;  break;
        case DRBOPT_NETWORK_TIMEOUT: *name = NULL,              *type = DRBTYPE_VAL;  break;
        case DRBOPT_INCL_MEDIA_INFO: *name = "size",            *type = DRBTYPE_BOOL; break;
        case DRBOPT_PATH_PREFIX:     *name = "to_path",         *type = DRBTYPE_PATH; break;
            
        default:
            return false; // Unknown option
    }
    return true;
}

/*!
 * \brief   Append an option to the arguments list.
 * \param[out]  args   arguments list
 * \param       name   option name
 * \param       value  option argument
 * \return  Error code (DRBERR_XXX).
 */
static int drbAppendOpt(char** args, char* name, char* value) {
    int err = DRBERR_OK;
    char* tmp;
    if(asprintf(&tmp, "%s&%s=%s", *args, name, value) != -1) {
        free(*args);
        *args = tmp;
    } else
        err = DRBERR_MALLOC;
    return err;
}


/*!
 * \brief   Set defined default regular arguments in the arguments list.
 * \param       cli    authenticated dropbox client
 * \param[out]  args   regular arguments list
 * \param       ra     regular default arguments to append (if defined)
 * \return  void
 */
static void drbSetDefaultRegularArgs(drbClient* cli, char** args, int ra) {
    drbOptType type;
    char *name, *value;
    for (int opt = 0; opt < DRBOPT_END; opt++) {
        if (ra & (1 << opt)) {
            drbGetOptAttr(opt, &name, &type);
            if(name && (value = cli->defaultOptions[opt].str) != NULL)
                drbAppendOpt(args, name, value);
        }
    }
}

/*!
 * \brief   Set defined default special arguments in the arguments list.
 * \param       cli     authenticated dropbox client
 * \param[out]  sArgs   special arguments list
 * \param       ra      special default arguements to set (if defined)
 * \return unset special arguements
 */
static int drbSetDefaultSpecialArgs(drbClient* cli, drbOptArg *sArgs, int sa) {
    for (int opt = 0; opt < DRBOPT_END; opt++) {
        int optBit = sa & (1 << opt);
        if (optBit && (opt == DRBOPT_NETWORK_TIMEOUT || cli->defaultOptions[opt].ptr)) {
            sa ^= optBit;
            switch (opt) {
                case DRBOPT_ROOT:
                    sArgs[DRBSHI_ROOT].str = drbStrDup(cli->defaultOptions[DRBOPT_ROOT].str);
                    break;
                case DRBOPT_PATH:
                    sArgs[DRBSHI_PATH].str = drbStrDup(cli->defaultOptions[DRBOPT_PATH].str);
                    break;
                case DRBOPT_NETWORK_TIMEOUT:
                    sArgs[DRBSHI_NETWORK_TIMEOUT] = cli->defaultOptions[DRBOPT_NETWORK_TIMEOUT];
                    break;
                case DRBOPT_IO_DATA:
                    sArgs[DRBSHI_IO_DATA] = cli->defaultOptions[DRBOPT_IO_DATA];
                    break;
                case DRBOPT_IO_FUNC:
                    sArgs[DRBSHI_IO_FUNC] = cli->defaultOptions[DRBOPT_IO_FUNC];
                    break;
            }
        }
    }
    return sa;
}

/*!
 * \brief   Parse all options and their arguments.
 *
 * Generic options parsing for API functions. There's two type of options:
 *   1. Function specific options:
 *      Argument value from these options are handled by the special handler
 *      function (sh) and all of them should appear in the va_list OR have a
 *      a default value.
 *
 *   2. Regular options:
 *      These options (with their values) are concatenated in the args string.
 *      As special args, they could either appear in the va_list or have a
 *      defined default value.
 *
 * \param       cli     authenticated dropbox client
 * \param       ap        va_list to parse
 * \param       sa        list of expected special options
 * \param       ra        list of possible regular options
 * \param[out]  args      arguments list, must be freed by the caller.
 * \param       sh        special handler function
 * \param       shArg     special handler argument
 * \return  Error code (DRBERR_XXX).
 */
static int drbGetOpt(drbClient* cli, va_list* ap, int sa, int ra, char** args,
                     int sh(int, va_list* ap, drbOptArg*, bool*), drbOptArg* shArg) {
    int opt, optBit;
    int err = DRBERR_OK;
    int parsedOpts = 0;
    bool ignored;
    drbOptArg arg;
    char *name;
    
    if ((*args = calloc(1, 1)) == NULL)
        err = DRBERR_MALLOC;
    
    while ((!err && (optBit = (1 << (opt = va_arg(*ap, int)))) != DRBBIT_END))
        if (!(optBit & parsedOpts)) { // if argument was not already parsed...
            parsedOpts ^= optBit;     // add it to the parsed list
            
            if (optBit & sa) {   // if argument need a special care...
                // and argument was handled without problem and was not ignored...
                if ((err = sh(optBit, ap, shArg, &ignored)) == DRBERR_OK && !ignored)
                    sa ^= optBit; // remove it from the list
            } else {
                if (optBit & ra)
                    ra ^= optBit;
                
                // set the opt string ID and his expected value type
                drbOptType type;
                if (! drbGetOptAttr(opt, &name, &type)) {
                    err = DRBERR_UNKNOWN_OPT;
                } else {
                    // Add the new argument to the arguments list string IF:
                    //   - there is no error so far
                    //   - drbGetOptArg parsed the argument with success
                    //   - the argument is not ignored
                    err = drbGetOptArg(ap, type, &arg, &ignored);
                    if (!err && !ignored) {
                        err = drbAppendOpt(args, name, arg.str);
                        free(arg.ptr);
                    }
                }
            }
        } else
            err = DRBERR_DUPLICATED_OPT;
    
    // Set the defined regular args
    drbSetDefaultRegularArgs(cli, args, ra);
    
    // if there's no error but special arguments remain...
    if (!err && drbSetDefaultSpecialArgs(cli, shArg, sa) != 0)
        err = DRBERR_MISSING_OPT;
    
    return err;
}


/*!
 * \brief   Handle special options.
 * \param    opt       option to handle
 * \param    ap        va_list to parse
 * \param[out]  shArg     handler argument
 * \param[out]  ignored   indicates whether the option is ignored or not
 * \return  Error code (DRBERR_XXX).
 */
static int specialHandler(int optBit, va_list* ap, drbOptArg* shArg, bool* ignored) {
    switch (optBit) {
        case DRBBIT_NETWORK_TIMEOUT:
            return drbGetOptArg(ap, DRBTYPE_STR, &shArg[DRBSHI_NETWORK_TIMEOUT], ignored);
        case DRBBIT_ROOT:
            return drbGetOptArg(ap, DRBTYPE_STR, &shArg[DRBSHI_ROOT], ignored);
        case DRBBIT_PATH:
            return drbGetOptArg(ap, DRBTYPE_PATH, &shArg[DRBSHI_PATH], ignored);
        case DRBBIT_IO_DATA:
            ((void**)shArg)[DRBSHI_IO_DATA] = va_arg(*ap, void*);
            *ignored = false; // a NULL data is allowed
            return DRBERR_OK;
        case DRBBIT_IO_FUNC:
            *ignored = (shArg[DRBSHI_IO_FUNC].ptr = va_arg(*ap, void*)) == NULL;
            return DRBERR_OK;
        default:
            *ignored = true;
            return DRBERR_UNKNOWN;
    }
}

/*!
 * \brief   Describe an error code with a string.
 * \param   code   code to describe
 * \return  Error description.
 */
static char* drbLocalError(int code) {
    char* message = NULL;
    switch (code) {
        case DRBERR_DUPLICATED_OPT: message = "Duplicated option";        break;
        case DRBERR_INVALID_VAL:    message = "Invalid value";            break;
        case DRBERR_MALLOC:         message = "Memory allocation failed"; break;
        case DRBERR_MISSING_OPT:    message = "Missing option";           break;
        case DRBERR_UNKNOWN_OPT:    message = "Unknown option";           break;
        case DRBERR_UNKNOWN:        message = "Unknown error";            break;
        case DRBERR_NETWORK:        message = "Network issue";            break;
        case DRBERR_TIMEOUT:        message = "Request timed out";        break;
    }
    return message ? drbStrDup(message) : NULL;
}

/*!
 * \brief   Set the right output (structure or error message).
 * \param       err           current error code
 * \param       str           json formated output message from dropbox
 * \param       drbParseFct   handle the json to structure conversion
 * \param[out]  output        output structure or message
 * \return  void
 */
static void drbSetOutput(int err, char* str, void* (*drbParseFct)(char *str), void** output) {
    if (output) {
        if (err) {
            if((*output = drbLocalError(err)) == NULL)
                *output = drbParseError(str);
        } else  *output = drbParseFct(str);
    }
}

int drbSetDefault(drbClient* cli, ...) {
    va_list ap;
    va_start(ap, cli);
    int opt, err = DRBERR_OK;
    bool ignored;
    while ((!err && (opt = va_arg(ap, int)) != DRBOPT_END)) {
        // set the opt string ID and his expected value type
        char *name; drbOptType type; drbOptArg arg;
        if (!drbGetOptAttr(opt, &name, &type)) {
            err = DRBERR_UNKNOWN_OPT;
        } else {
            err = drbGetOptArg(&ap, type, &arg, &ignored);
            if (!err) {
                if (type != DRBTYPE_VAL)
                    free(cli->defaultOptions[opt].ptr);
                cli->defaultOptions[opt].str = ignored ? NULL : arg.ptr;
            }
        }
    }
    
    va_end(ap);
    return err;
}

void drbDestroyMetadata(drbMetadata* meta, bool withList) {
    if (meta) {
        free(meta->hash);
        free(meta->rev);
        free(meta->thumbExists);
        free(meta->bytes);
        free(meta->modified);
        free(meta->path);
        free(meta->isDir);
        free(meta->icon);
        free(meta->root);
        free(meta->size);
        free(meta->clientMtime);
        free(meta->isDeleted);
        free(meta->mimeType);
        free(meta->revision);
        
        if (withList)
            drbDestroyMetadataList(meta->contents, true);
        free(meta);
    }
}

void drbDestroyMetadataList(drbMetadataList* list, bool withMetadata) {
    if (list) {
        if (withMetadata)
            for (int i = 0; i < list->size; i++)
                drbDestroyMetadata(list->array[i], true);
        free(list->array);
        free(list);
    }
}

void drbDestroyCopyRef(drbCopyRef* ref) {
    if (ref) {
        free(ref->copyRef);
        free(ref->expires);
        free(ref);
    }
}

void drbDestroyLink(drbLink* link) {
    if (link) {
        free(link->url);
        free(link->expires);
        free(link);
    }
}

void drbDestroyAccountInfo(drbAccountInfo* info) {
    if (info) {
        free(info->referralLink);
        free(info->displayName);
        free(info->uid);
        free(info->country);
        free(info->email);
        free(info->quotaInfo.datastores);
        free(info->quotaInfo.shared);
        free(info->quotaInfo.quota);
        free(info->quotaInfo.normal);
        free(info);
    }
}

void drbDestroyDelta(drbDelta* delta, bool withMetadata) {
    if (delta) {
        free(delta->reset);
        free(delta->cursor);
        free(delta->hasMore);
        for (int i = 0; i < delta->entries.size; i++) {
            free(delta->entries.array[i].path);
            if (withMetadata)
                drbDestroyMetadata(delta->entries.array[i].metadata, true);
        }
        free(delta->entries.array);
        free(delta);
    }
}

void drbDestroyPollDelta(drbPollDelta* poll) {
    if (poll) {
        free(poll->changes);
        free(poll->backoff);
        free(poll);
    }
}

drbClient* drbCreateClient(const char* cKey, const char* cSecret, const char* tKey, const char* tSecret) {
    drbClient* cli = NULL;
    if (cKey && cSecret) {
        if((cli = malloc(sizeof(drbClient))) != NULL) {
            cli->c.key = strdup(cKey);
            cli->c.secret = strdup(cSecret);
            
            cli->t.key = drbStrDup(tKey);
            cli->t.secret = drbStrDup(tSecret);
            
            memset(cli->defaultOptions, 0, sizeof(drbOptArg) * DRBOPT_END);
        }
    }
    return cli;
}

void drbDestroyClient(drbClient* cli) {
    if (cli) {
        free(cli->c.key);
        free(cli->c.secret);
        free(cli->t.key);
        free(cli->t.secret);
        
        for (int opt = 0; opt < DRBOPT_END; opt++) {
            char *name; drbOptType type;
            drbGetOptAttr(opt, &name, &type);
            
            // Values and external pointers are not be freed
            if (type != DRBTYPE_VAL && type != DRBTYPE_PTR)
                if (cli->defaultOptions[opt].ptr)
                    free(cli->defaultOptions[opt].ptr);
        }
        free(cli);
    }
}

void drbInit() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void drbCleanup() {
    curl_global_cleanup();
}

drbOAuthToken* drbObtainRequestToken(drbClient* cli) {
    drbOAuthToken* token = NULL;
    char *tKey, *tSecret;
    memStream answer; memStreamInit(&answer);
    drbOAuthPost(cli, DRBURI_REQUEST, &answer, memStreamWrite, 0);
    
    if (answer.data) {
        if (drbParseOauthTokenReply((char*)answer.data, &tKey, &tSecret)) {
            free(cli->t.key),    cli->t.key    = tKey;
            free(cli->t.secret), cli->t.secret = tSecret;
            token = &cli->t;
        }
        memStreamCleanup(&answer);
    }
    
    return token;
}

char* drbBuildAuthorizeUrl(drbOAuthToken* reqTok) {
    char* url = NULL;
    asprintf(&url, "%s?oauth_token=%s", DRBURI_AUTHORIZATION, reqTok->key);
    return url;
}

drbOAuthToken* drbObtainAccessToken(drbClient* cli) {
    drbOAuthToken* token = NULL;
    char *tKey, *tSecret;
    memStream answer; memStreamInit(&answer);
    drbOAuthPost(cli, DRBURI_ACCESS, &answer, memStreamWrite, 0);
    
    if (answer.data) {
        if (drbParseOauthTokenReply((char*)answer.data, &tKey, &tSecret)) {
            free(cli->t.key),    cli->t.key    = tKey;
            free(cli->t.secret), cli->t.secret = tSecret;
            token = &cli->t;
        }
        memStreamCleanup(&answer);
    }
    
    return token;
}

int drbGetAccountInfo(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[1]; memset(sArgs, 0, 1 * sizeof(drbOptArg));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_ACC_INFO, DRBRA_ACC_INFO, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
        int res = asprintf(&url, "%s%s", DRBURI_ACCOUNT_INFO, args);
        if(res != -1) {
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    
    drbSetOutput(err, answer.data, (void*)drbParseAccountInfo, output);
    
    memStreamCleanup(&answer);
    free(args);
    
    return err;
}

int drbGetMetadata(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[3]; memset(sArgs, 0, 3 * sizeof(drbOptArg));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_METADATA, DRBRA_METADATA, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
        int res = asprintf(&url, "%s/%s%s?%s", DRBURI_METADATA,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            err = drbOAuthGet(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    
    drbSetOutput(err, answer.data, (void*)drbParseMetadata, output);
    
    memStreamCleanup(&answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;
}

int drbGetFile(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[5]; memset(sArgs, 0, 5 * sizeof(void*));
    char *answer = NULL;
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_GET_FILES, DRBRA_GET_FILES, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
        int res = asprintf(&url,"%s/%s%s%s", DRBURI_GET_FILES,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            err = drbOAuthGetFile(cli, url, sArgs[DRBSHI_IO_DATA].ptr,
                                  sArgs[DRBSHI_IO_FUNC].ptr,
                                  output ? &answer : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer, (void*)drbParseMetadata, output);
    
    free(answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    
    return err;
}

int drbGetRevisions(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[3]; memset(sArgs, 0, 3 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_REVISIONS, DRBRA_REVISIONS, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
        int res = asprintf(&url,"%s/%s%s%s", DRBURI_REVISIONS,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            err = drbOAuthGet(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbStrParseMetadataList, output);
    memStreamCleanup(&answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;
}

int drbSearch(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[3]; memset(sArgs, 0, 3 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_SEARCH, DRBRA_SEARCH, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
        int res = asprintf(&url,"%s/%s%s%s", DRBURI_SEARCH,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            err = drbOAuthGet(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbStrParseMetadataList, output);
    
    memStreamCleanup(&answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;
}

int drbGetThumbnail(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    char *answer = NULL;
    drbOptArg sArgs[5]; memset(sArgs, 0, 5 * sizeof(void*));
    
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_THUMBNAILS, DRBRA_THUMBNAILS, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s/%s%s%s", DRBURI_THUMBNAILS,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthGetFile(cli, url, sArgs[DRBSHI_IO_DATA].ptr,
                                  sArgs[DRBSHI_IO_FUNC].ptr,
                                  output ? &answer : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    
    drbSetOutput(err, answer, (void*)drbParseMetadata, output);
    
    free(answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    
    return err;
}

int drbCopy(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[1]; memset(sArgs, 0, 1 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_COPY, DRBRA_COPY, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s%s", DRBURI_COPY, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbParseMetadata, output);
    memStreamCleanup(&answer);
    free(args);
    return err;
}

int drbCreateFolder(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[1]; memset(sArgs, 0, 1 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_CREATE_FOLDER, DRBRA_CREATE_FOLDER, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        
        int res = asprintf(&url,"%s%s", DRBURI_CREATE_FOLDER, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    
    drbSetOutput(err, answer.data, (void*)drbParseMetadata, output);
    memStreamCleanup(&answer);
    free(args);
    return err;
}

int drbDelete(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[1]; memset(sArgs, 0, 1 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_DELETE, DRBRA_DELETE, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s%s", DRBURI_DELETE, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
        
    }
    drbSetOutput(err, answer.data, (void*)drbParseMetadata, output);
    memStreamCleanup(&answer);
    free(args);
    return err;
}

int drbMove(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[1]; memset(sArgs, 0, 1 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_MOVE, DRBRA_MOVE, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s%s", DRBURI_MOVE, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbParseMetadata, output);
    memStreamCleanup(&answer);
    free(args);
    return err;
}

int drbGetDelta(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[1]; memset(sArgs, 0, 1 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_DELTA, DRBRA_DELTA, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s?%s", DRBURI_DELTA, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    
    drbSetOutput(err, answer.data, (void*)drbParseDelta, output);
    memStreamCleanup(&answer);
    free(args);
    return err;
}

int drbRestore(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[3]; memset(sArgs, 0, 3 * sizeof(void*));
    memStream answer;   memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_RESTORE, DRBRA_RESTORE, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s/%s%s%s", DRBURI_RESTORE, sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbParseMetadata, output);
    memStreamCleanup(&answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;
}

int drbShare(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[3];   memset(sArgs, 0, 3 * sizeof(void*));
    memStream answer; memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_SHARES, DRBRA_SHARES, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s/%s%s%s", DRBURI_SHARES,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbParseLink, output);
    memStreamCleanup(&answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;
}

int drbGetMedia(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[3];   memset(sArgs, 0, 3 * sizeof(void*));
    memStream answer; memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_MEDIA, DRBRA_MEDIA, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s/%s%s?%s", DRBURI_MEDIA,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPost(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbParseLink, output);
    memStreamCleanup(&answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;
}

int drbGetCopyRef(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[3];   memset(sArgs, 0, 3 * sizeof(void*));
    memStream answer; memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_COPY_REF, DRBRA_COPY_REF, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s/%s%s%s", DRBURI_COPY_REF,
                           sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthGet(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer.data, (void*)drbParseCopyRef, output);
    memStreamCleanup(&answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;  
}

int drbPutFile(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[5]; memset(sArgs, 0, 5 * sizeof(void*));
    char *answer = NULL;
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_PUT_FILES, DRBRA_PUT_FILES, &args, specialHandler, sArgs);
    va_end(ap);  
    
    if (!err) {
        int res =  asprintf(&url, "%s/%s%s%s", DRBURI_PUT_FILES,
                            sArgs[DRBSHI_ROOT].str, sArgs[DRBSHI_PATH].str, args);
        if (res != -1) {      
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthPostFile(cli, url, sArgs[DRBSHI_IO_DATA].ptr,
                                   sArgs[DRBSHI_IO_FUNC].ptr,
                                   output ? &answer : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    drbSetOutput(err, answer, (void*)drbParseMetadata, output);
    free(answer);
    free(args),  free(sArgs[DRBSHI_ROOT].str), free(sArgs[DRBSHI_PATH].str);
    return err;
}

int drbLongPollDelta(drbClient* cli, void** output, ...) {
    char *args = NULL;
    char *url = NULL;
    drbOptArg sArgs[1];   memset(sArgs, 0, 1 * sizeof(void*));
    memStream answer; memStreamInit(&answer);
    
    va_list ap;
    va_start(ap, output);
    int err = drbGetOpt(cli, &ap, DRBSA_LONGPOLL_DELTA, DRBRA_LONGPOLL_DELTA, &args, specialHandler, sArgs);
    va_end(ap);
    
    if (!err) {
        int res = asprintf(&url,"%s?%s", DRBURI_LONGPOLL_DELTA, args+1);
        if (res != -1) {
            int timeout = sArgs[DRBSHI_NETWORK_TIMEOUT].value;
            err = drbOAuthGet(cli, url, &answer, output ? memStreamWrite : NULL, timeout);
            free(url);
        } else
            err = DRBERR_MALLOC;
    }
    
    drbSetOutput(err, answer.data, (void*)drbParsePollDelta, output);
    memStreamCleanup(&answer);
    free(args);
    return err;
}
