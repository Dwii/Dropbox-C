/*!
 * \file    dropboxOAuth.h
 * \brief   OAuth library for dropbox library.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#ifndef DROPBOX_OAUTH_H
#define DROPBOX_OAUTH_H

#include <stdbool.h>
#include "dropbox.h"
#include "dropboxUtils.h"

typedef union {
    void* ptr;
    char* str;
    int value;
} drbOptArg;

/*!
 * \struct  drbClient
 * \breif   Client data used the whole time.
 */
struct drbClient{
    drbOAuthToken c;
    drbOAuthToken t;
    drbOptArg defaultOptions[DRBOPT_END];
};

int drbOAuthGet(drbClient* cli, const char* url, void* data, void* writeFct, int timeout);
int drbOAuthPost(drbClient* cli, const char* url, void* data, void* writeFct, int timeout);

int drbOAuthGetFile(drbClient* cli, const char* url, void* data, void* writeFct, char** answer, int timeout);
int drbOAuthPostFile(drbClient* cli, const char *url, void* data,
                     ssize_t (*readFct)(void *, size_t , size_t , void *),
                     char** answer, int timeout);
char *drbEncodePath(const char *string);
bool drbParseOauthTokenReply(const char *answer, char **key, char **secret);

#endif /* DROPBOX_OAUTH_H */