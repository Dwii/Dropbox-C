/*!
 * \file    dropboxJson.c
 * \brief   Json parsing library for dropbox library structures.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#define _GNU_SOURCE

#include <stdbool.h>
#include <string.h>
#include <jansson.h>
#include "dropboxJson.h"

static void drbJsonParseMetadata(json_t *root, drbMetadata* meta);

/*!
 * \brief   Parse a JSON string entry.
 * \param   root   JSON root node
 * \param   key    key (name) of the entry to parse
 * \return  parsed string (must be freed by called)
 */
static char* drbJsonGetStr(json_t* root, char* key) {
    char* pStr = NULL;
    if(json_unpack(root, "{ss}", key, &pStr) != -1) {
        pStr = strdup(pStr);
    }
    return pStr;
}

/*!
 * \brief   Parse a JSON integer entry.
 * \param   root   JSON root node
 * \param   key    key (name) of the entry to parse
 * \return  pointer to the parsed integer (must be freed by caller)
 */
static unsigned int* drbJsonGetInt(json_t* root, char* key) {
    unsigned int* pInt = NULL;
    unsigned int value;
    if(json_unpack(root, "{si}", key, &value) != -1) {
        if((pInt = malloc(sizeof(unsigned int))) != NULL) {
            *pInt = value;
        }
    }
    return pInt;
}

/*!
 * \brief   Parse a JSON boolean entry.
 * \param   root   JSON root node
 * \param   key    key (name) of the entry to parse
 * \return  pointer to the parsed boolean (must be freed by caller)
 */
static bool* drbJsonGetBool(json_t* root, char* key) {
    bool* pBool = NULL;
    unsigned int value;
    if(json_unpack(root, "{sb}", key, &value) != -1) {
        if((pBool = malloc(sizeof(bool))) != NULL) {
            *pBool = value;
        }
    }
    return pBool;
}

/*!
 * \brief   Parse a JSON metadata list entry.
 * \param       root   JSON root node that should contain the metadata list
 * \param[out]  list   list to load with JSON data
 * \return  void
 */
static void drbJsonParseMetadataList(json_t *root, drbMetadataList* list) {
    memset(list, 0, sizeof(drbMetadataList));
    if (root && json_is_array(root)) {
        list->size = json_array_size(root);
        if(list->size > 0) {
            if((list->array = malloc(sizeof(drbMetadata*)*list->size)) != NULL) {
                for (int i = 0; i < list->size; i++) {
                    json_t* meta = json_array_get(root, i);
                    list->array[i] = malloc(sizeof(drbMetadata));
                    drbJsonParseMetadata(meta, list->array[i]);
                }
            }
        }
    }
}

/*!
 * \brief   Parse a JSON metadata entry.
 * \param       root   JSON root node that should contain the metadata
 * \param[out]  meta   metadata to load with JSON data
 * \return  void
 */
static void drbJsonParseMetadata(json_t *root, drbMetadata* meta) {
    if (root) {
        memset(meta, 0, sizeof(drbMetadata));
        meta->hash        = drbJsonGetStr (root, "hash");
        meta->rev         = drbJsonGetStr (root, "rev");
        meta->thumbExists = drbJsonGetBool(root, "thumb_exists");
        meta->bytes       = drbJsonGetInt (root, "bytes");
        meta->modified    = drbJsonGetStr (root, "modified");
        meta->path        = drbJsonGetStr (root, "path");
        meta->isDir       = drbJsonGetBool(root, "is_dir");
        meta->icon        = drbJsonGetStr (root, "icon");
        meta->root        = drbJsonGetStr (root, "root");
        meta->size        = drbJsonGetStr (root, "size");
        meta->clientMtime = drbJsonGetStr (root, "client_mtime");
        meta->isDeleted   = drbJsonGetBool(root, "is_deleted");
        meta->mimeType    = drbJsonGetStr (root, "mime_type");
        meta->revision    = drbJsonGetInt (root, "revision");
        
        json_t *contents = json_object_get(root, "contents");
        if (contents) {
            if((meta->contents = malloc(sizeof(drbMetadataList))) != NULL) {
                drbJsonParseMetadataList(contents, meta->contents);
            }
        }
    }
}

/*!
 * \brief   Parse a JSON delta entry.
 * \param       root    JSON root node that should contain the delta entry
 * \param[out]  entry   entry to load with JSON data
 * \return  void
 */
static void drbJsonParseDeltaEntry(json_t *root, drbDeltaEntry* entry) {
    
    if (root && json_is_array(root) && json_array_size(root) == 2) {
        // Get path
        json_t* path = json_array_get(root, 0);
        if (path && json_is_string(path))
            entry->path = strdup(json_string_value(path));
        
        // Get metadata
        json_t* meta = json_array_get(root, 1);
        if (meta) {
            if ((entry->metadata = malloc(sizeof(drbMetadata))) != NULL) {
                drbJsonParseMetadata(meta, entry->metadata);
            }
        }
    }
}

/*!
 * \brief   Parse a JSON formated text error entry.
 * \param   str   JSON formated error text
 * \return  extracted error message
 */
char* drbParseError(char* str) {
    char* error = NULL;
    json_t *root = json_loads(str, 0, NULL);
    if (root){
        error = drbJsonGetStr(root, "error");
        json_decref(root);
    }
    return error;
}


/*!
 * \brief   Create a drbCopyRef and load it from a JSON formated text.
 * \param   str   JSON formated text to load in the new drbCopyRef
 * \return  created and loaded drbCopyRef pointer
 */
drbCopyRef* drbParseCopyRef(char* str) {
    drbCopyRef* ref = NULL;
    json_t *root = json_loads(str, 0, NULL);
    if (root) {
        if((ref = calloc(1, sizeof(drbCopyRef))) != NULL) {
            ref->copyRef = drbJsonGetStr (root, "copy_ref");
            ref->expires = drbJsonGetStr (root, "expires");
        }
        json_decref(root);
    }
    return ref;
}

/*!
 * \brief   Create a drbParseLink and load it from a JSON formated text.
 * \param   str   JSON formated text to load in the new drbParseLink
 * \return  created and loaded drbLink pointer
 */
drbLink* drbParseLink(char* str) {
    json_t *root = json_loads(str, 0, NULL);
    drbLink* link = NULL;
    if (root) {
        if((link = calloc(1, sizeof(drbLink))) != NULL) {
            link->url     = drbJsonGetStr (root, "url");
            link->expires = drbJsonGetStr (root, "expires");
        }
        json_decref(root);
    }
    return link;
}

/*!
 * \brief   Create a drbMetadataList and load it from a JSON formated text.
 * \param   str   JSON formated text to load in the new drbMetadataList
 * \return  created and loaded drbMetadataList pointer
 */
drbMetadataList* drbStrParseMetadataList(char* str) {
    drbMetadataList* list = NULL;
    json_t *root = json_loads(str, 0, NULL);
    if (root) {
        if((list = malloc(sizeof(drbMetadataList))) != NULL) {
            drbJsonParseMetadataList(root, list);
        }
        json_decref(root);
    }
    return list;
}

/*!
 * \brief   Create a drbMetadata and load it from a JSON formated text.
 * \param   str   JSON formated text to load in the new drbMetadata
 * \return  created and loaded drbMetadata pointer
 */
drbMetadata* drbParseMetadata(char* str) {
    drbMetadata* meta = NULL;
    json_loads(NULL, 0, NULL);
    json_t *root = json_loads(str, 0, NULL);
    if (root) {
        if((meta = malloc(sizeof(drbMetadata))) != NULL) {
            drbJsonParseMetadata(root, meta);
        }
        json_decref(root);
    }
    return meta;
}

/*!
 * \brief   Create a drbAccountInfo and load it from a JSON formated text.
 * \param   str   JSON formated text to load in the new drbAccountInfo
 * \return  created and loaded drbAccountInfo pointer
 */
drbAccountInfo* drbParseAccountInfo(char* str) {
    drbAccountInfo* info = NULL;
    json_t *root = json_loads(str, 0, NULL);
    if (root) {
        if((info = calloc(1, sizeof(drbAccountInfo))) != NULL) {
            info->referralLink = drbJsonGetStr(root, "referral_link");
            info->displayName  = drbJsonGetStr(root, "display_name");
            info->uid          = drbJsonGetInt(root, "uid");
            info->country      = drbJsonGetStr(root, "country");
            info->email        = drbJsonGetStr(root, "email");
            
            json_t *quota = json_object_get(root, "quota_info");
            if(quota) {
                info->quotaInfo.datastores = drbJsonGetInt(quota, "datastores");
                info->quotaInfo.shared     = drbJsonGetInt(quota, "shared");
                info->quotaInfo.quota      = drbJsonGetInt(quota, "quota");
                info->quotaInfo.normal     = drbJsonGetInt(quota, "normal");
            }
        }
        json_decref(root);
    }
    return info;
}


/*!
 * \brief   Create a drbDelta and load it from a JSON formated text.
 * \param   str   JSON formated text to load in the new drbDelta
 * \return  created and loaded drbDelta pointer
 */
drbDelta* drbParseDelta(char* str) {
    drbDelta* delta = NULL;
    json_t *root = json_loads(str, 0, NULL);
    
    if (root) {
        if((delta = calloc(1, sizeof(drbDelta))) != NULL) {
            delta->reset   = drbJsonGetBool(root, "reset");
            delta->cursor  = drbJsonGetStr (root, "cursor");
            delta->hasMore = drbJsonGetBool(root, "has_more");
            
            json_t *entries = json_object_get(root, "entries");
            
            if (entries && json_is_array(entries)) {
                size_t size = json_array_size(entries);
                if (size > 0) {
                    delta->entries.array = calloc(size, sizeof(drbDeltaEntry));
                    if (delta->entries.array) {
                        delta->entries.size = size;
                        for (int i = 0; i < delta->entries.size; i++) {
                            json_t *entry = json_array_get(entries, i);
                            drbJsonParseDeltaEntry(entry, &delta->entries.array[i]);
                        }
                    }
                }
            }
        }
        json_decref(root);
    }
    return delta;
}

/*!
 * \brief   Create a drbPollDelta and load it from a JSON formated text.
 * \param   str   JSON formated text to load in the new drbPollDelta
 * \return  created and loaded drbPollDelta pointer
 */
drbPollDelta* drbParsePollDelta(char* str) {
    drbPollDelta* poll = NULL;
    json_t *root = json_loads(str, 0, NULL);
    
    if (root) {
        if((poll = calloc(1, sizeof(drbPollDelta))) != NULL) {
            poll->changes = drbJsonGetBool(root, "changes");
            poll->backoff = drbJsonGetInt(root, "backoff");
        }
        json_decref(root);
    }
    
    return poll;
}