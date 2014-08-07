/*!
 * \file    dropboxJson.h
 * \brief   Json parsing library for dropbox library structures.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#ifndef DROPBOX_JSON_H
#define DROPBOX_JSON_H

#include "dropbox.h"

char* drbParseError(char* str);
drbCopyRef* drbParseCopyRef(char* str);
drbLink* drbParseLink(char* str);
drbMetadataList* drbParseMetadataList(char* str);
drbMetadata* drbParseMetadata(char* str);
drbAccountInfo* drbParseAccountInfo(char* str);
drbDelta* drbParseDelta(char* str);
drbMetadataList* drbStrParseMetadataList(char *str);
drbPollDelta* drbParsePollDelta(char* str);

#endif /* DROPBOX_JSON_H */