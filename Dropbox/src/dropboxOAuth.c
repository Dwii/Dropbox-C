/*!
 * \file    dropboxOAuth.c
 * \brief   OAuth library for dropbox library.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <oauth.h>
#include <curl/curl.h>
#include <memStream.h>
#include "dropboxOAuth.h"
#include "dropboxUtils.h"

static const char* DRB_HEADER_FIELD_METADATA = "x-dropbox-metadata";

typedef enum {
    DRB_HTTP_GET = 0,
    DRB_HTTP_POST1,
    DRB_HTTP_POST2,
} drbHttpMethod;


typedef struct {
    size_t (*fct)(const void*, size_t, size_t, void *);
    void* data;
} drbWrappedIO;

typedef struct {
    drbWrappedIO ok;  /*!< IO used when http code == 200. */
    drbWrappedIO ko;  /*!< IO used when http code != 200. */
    drbWrappedIO* io; /*!< Must be initialized to NULL. */
    CURL* curl;       /*!< CURL handle to query for the http call. */
} drbWrappedIOData;

/*!
 * \brief   Translate a Curl error code to a DRBERR_XXX.
 * \param   code   code to translate.
 * \return  translated code.
 */
static int drbErrorFromCurl(CURLcode code) {
    switch (code) {
        case CURLE_OUT_OF_MEMORY:         return DRBERR_MALLOC;
        case CURLE_COULDNT_RESOLVE_PROXY:
        case CURLE_COULDNT_RESOLVE_HOST:
        case CURLE_COULDNT_CONNECT:
        case CURLE_PARTIAL_FILE:          return DRBERR_NETWORK;
        case CURLE_OPERATION_TIMEDOUT:    return DRBERR_TIMEOUT;
        default:                          return DRBERR_UNKNOWN;
    }
}

/*!
 *   Act as an IO call (read or write), but do absolutely nothing.
 */
static size_t drbNullIOCall(const void *ptr, size_t size, size_t count, void *mem) {
    return size * count;
}

/*!
 *   Depending on the http code of the curl request, decide write with the 'ok'
 *   wrapped function or the 'ko' (in case of error).
 */
static size_t drbWrappedIOCall(const void *ptr, size_t size, size_t count, drbWrappedIOData *data) {
    if (!data->io) {
        long httpCode;
        curl_easy_getinfo (data->curl, CURLINFO_RESPONSE_CODE, &httpCode);
        data->io = (httpCode == 200 ? &data->ok : &data->ko);
    }
    return data->io->fct(ptr, size, count, data->io->data);
}


/*!
 * \brief   Found a copy the OAuth key and secret from the server answer.
 * \param       answer   server raw answer
 * \param[out]  key      char* where the OAuth key should be stored
 * \param[out]  secret   char* where the OAuth secret should be stored
 * \return  indicates whether the key and the secret were founded or not.
 */
bool drbParseOauthTokenReply(const char *answer, char **key, char **secret) {
    bool ok = true;
    char **rv = NULL;
    char* params[] = { "oauth_token=", "oauth_token_secret=" };
    
    *key = *secret = NULL;
    
    int nbParam = sizeof(params) / sizeof(params[0]);
    
    int rc = oauth_split_url_parameters(answer, &rv);
    qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
    
    if (rc >= nbParam) {
        int *paramIndex, j = 0;
        size_t paramLen = strlen(params[j]);
        if ((paramIndex = malloc(nbParam * sizeof(int))) != NULL) {
            for (int i = 0; i < rc && j < nbParam; i++)
                if (!strncmp(params[j], rv[i], paramLen)) {
                    paramIndex[j++] = i;
                    if (j < nbParam)
                        paramLen = strlen(params[j]);
                }
            if (j == nbParam) {
                if (key)    *key    = strdup(&(rv[paramIndex[0]][12]));
                if (secret) *secret = strdup(&(rv[paramIndex[1]][19]));
            }
            free(paramIndex);
        }else  ok = false;
    } else  ok = false;
    
    if (!ok) {
        free(*key), *key = NULL;
        free(*secret), *secret = NULL;
    }
    
    free(rv);
    return ok;
}

/*!
 * \brief   Encode a path string to be OAuth complient.
 *
 * This is just a slight modification of oauth_url_escape from liboauth library:
 *   -# '/' is not escaped anymore
 *   -# memory allocation is checked
 *   -# code aspect was modified to match the library coding convention
 *
 * \param   path   path to encode
 * \return  encoded path (must be freed by caller)
 */
char* drbEncodePath(const char *path) {
    size_t alloc, newLength, length, i = 0;
    
    newLength = alloc = ( length = (path ? strlen(path) : 0) ) + 1;
    
    char *encPath = (char*) malloc(alloc);
    
    // loop while string end isnt reached and while ns is a valid pointer
    while(length-- && encPath) {
        unsigned char in = *path++;
        
        switch(in){
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case 'a': case 'b': case 'c': case 'd': case 'e':
            case 'f': case 'g': case 'h': case 'i': case 'j':
            case 'k': case 'l': case 'm': case 'n': case 'o':
            case 'p': case 'q': case 'r': case 's': case 't':
            case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            case 'A': case 'B': case 'C': case 'D': case 'E':
            case 'F': case 'G': case 'H': case 'I': case 'J':
            case 'K': case 'L': case 'M': case 'N': case 'O':
            case 'P': case 'Q': case 'R': case 'S': case 'T':
            case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            case '_': case '~': case '.': case '-': case '/':
                encPath[i++] = in;
                break;
            default:
                newLength += 2; /* this'll become a %XX */
                if(newLength > alloc) {
                    alloc *= 2;
                    if((encPath = memRealloc(encPath, alloc)) == NULL)
                        continue;
                }
                snprintf(&encPath[i], 4, "%%%02X", in);
                i += 3;
                break;
        }
    }
    
    // Shrink and finilaze the new string if ns is a valid pointer
    if (encPath && (encPath = memRealloc(encPath, newLength)) != NULL) {
        encPath[i] = '\0';
    }
    
    return encPath;
}

/*!
 * \brief   Perform an OAuth GET or POST request for a Dropbox client.
 * \param        curl       curl session
 * \param        cli        authenticated dropbox client
 * \param        url        request base url
 * \param        method     DRB_HTTP_GET or DRB_HTTP_POST
 * \param        data       where the request answer is written (e.g. FILE*)
 * \param        writeFct   called function to write in data (e.g. fwrite)
 * \param[out]   header     server answer header (must be freed by caller)
 * \param        timeout    request timeout limit (0 is infinite)
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
static int drbOAuthCurlPerform(drbClient* cli, CURL *curl, const char* url,
                               drbHttpMethod method, void* data, void* writeFct,
                               char** header, int timeout)
{
    int err = DRBERR_OK;
    
    long httpCode = 0;
    char* postArg = NULL;
    char* reqUrl = oauth_sign_url2(url, method ? &postArg : NULL, OA_HMAC, NULL,
                                   cli->c.key, cli->c.secret,
                                   cli->t.key, cli->t.secret);
    
    memStream headerData; memStreamInit(&headerData);
    
    if(method) {
        if (postArg){
            if(method == DRB_HTTP_POST2) {
                char* tmpUrl;
                asprintf(&tmpUrl,"%s?%s", reqUrl, postArg);
                free(reqUrl);
                reqUrl = tmpUrl;
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
            } else if(method == DRB_HTTP_POST1) {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postArg);
            } else
                err = DRBERR_UNKNOWN;
        } else
            err = DRBERR_MALLOC;
    }
    
    if(!err) {
        curl_easy_setopt(curl, CURLOPT_URL, reqUrl);
        
        // Only write the answer if there's at least the write function
        if (writeFct) {
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFct);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
        } else {
            // Sadly CURLOPT_NOBODY leads to a 401 error, and when
            // CURLOPT_WRITEFUNCTION is unset, data are written in stdout, so...
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, drbNullIOCall);
        }
        
        if (header) { // Only read the header if it's needed
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerData);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, memStreamWrite);
        }
        
        // General curl options
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); // thread safe requirement
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        
        CURLcode curlCode = curl_easy_perform(curl);
        if (curlCode == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            
            if (header)
                *header = drbGetHeaderFieldContent(DRB_HEADER_FIELD_METADATA, headerData.data);
            
            if (httpCode != 200) {
                err = (int)httpCode;
            }
        } else {
            err = drbErrorFromCurl(curlCode);
        }
    }
    
    free(headerData.data);
    free(postArg);
    free(reqUrl);
    
    return err;
}

/*!
 * \brief   Get a dropbox file with an OAuth on a Dropbox client.
 * \param        cli        authenticated dropbox client
 * \param        url        request base url
 * \param        data       where the request answer is written (e.g. FILE*)
 * \param        writeFct   called function to write in data (e.g. fwrite)
 * \param[out]   answer     server answer (must be freed by caller)
 * \param        timeout    request timeout limit (0 is infinite)
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbOAuthGetFile(drbClient* cli, const char* url, void* data, void* writeFct,
                    char** answer, int timeout)
{
    
    int err;
    CURL *curl = curl_easy_init();
    
    if (curl) {
        memStream mem; memStreamInit(&mem);
        void* koWriteFct = answer? (void*)memStreamWrite : (void*)drbNullIOCall;
        drbWrappedIOData ioData;
        ioData.curl = curl;
        ioData.ok.data = data, ioData.ok.fct = writeFct;
        ioData.ko.data = &mem, ioData.ko.fct = koWriteFct;
        ioData.io = NULL;
        
        char* header = NULL;
        err = drbOAuthCurlPerform(cli, curl, url, DRB_HTTP_GET, &ioData,
                                  drbWrappedIOCall, answer ? &header : NULL,
                                  timeout);
        curl_easy_cleanup(curl);
        if(!err || err > 200) {
            if (answer) {
                if(err) {
                    *answer = mem.data;
                    free(header);
                } else {
                    *answer = header;
                }
            }
        }
    } else
        err = DRBERR_MALLOC;
    
    return err;
}

/*!
 * \brief   Perform an OAuth GET request for a Dropbox client.
 * \param        cli        authenticated dropbox client
 * \param        url        request base url
 * \param        data       where the request answer is written (e.g. FILE*)
 * \param        writeFct   called function to write in data (e.g. fwrite)
 * \param[out]   header     server answer header (must be freed by caller)
 * \param        timeout    request timeout limit (0 is infinite)
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbOAuthGet(drbClient* cli, const char* url, void* data, void* writeFct, int timeout) {
    int err;
    CURL *curl = curl_easy_init();
    if (curl) {
        err = drbOAuthCurlPerform(cli, curl, url, DRB_HTTP_GET, data, writeFct, NULL, timeout);
        curl_easy_cleanup(curl);
    } else
        err = DRBERR_MALLOC;
    return err;
}

/*!
 * \brief   Perform an OAuth POST request for a Dropbox client.
 * \param        cli        authenticated dropbox client
 * \param        url        request base url
 * \param        data       where the request answer is written (e.g. FILE*)
 * \param        writeFct   called function to write in data (e.g. fwrite)
 * \param[out]   header     server answer header (must be freed by caller)
 * \param        timeout    request timeout limit (0 is infinite)
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbOAuthPost(drbClient* cli, const char* url, void* data, void* writeFct, int timeout) {
    int err;
    CURL *curl = curl_easy_init();
    if (curl) {
        err = drbOAuthCurlPerform(cli, curl, url, DRB_HTTP_POST1, data, writeFct, NULL, timeout);
        curl_easy_cleanup(curl);
    } else
        err = DRBERR_MALLOC;
    
    return err;
}

/*!
 * \brief   Perform an OAuth GET or POST request for a Dropbox client.
 * \param        cli        authenticated dropbox client
 * \param        url        request base url
 * \param        data       where the request answer is readed (e.g. FILE*)
 * \param        readFct    called function to read data content (e.g. fwrite)
 * \param[out]   answer     server answer (must be freed by caller)
 * \param        timeout    request timeout limit (0 is infinite)
 * \return  error code (DRBERR_XXX or http error returned by the Dropbox server)
 */
int drbOAuthPostFile(drbClient* cli, const char *url, void* data,
                     ssize_t (*readFct)(void *, size_t , size_t , void *),
                     char** answer, int timeout)
{
    
    int err;
    
    memStream fileData; memStreamInit(&fileData);
    if (memStreamLoad(&fileData, data, (void*)readFct)) {
        
        // Build request url
        char *reqUrl = NULL;
        char* key = oauth_catenc(2, cli->c.secret, cli->t.secret);
        char* sign = oauth_sign_hmac_sha1_raw(fileData.data, fileData.size, key, strlen(key));
        asprintf(&reqUrl,"%s&xoauth_body_signature=%s&param=val"
                 "&xoauth_body_signature_method=HMAC_SHA1", url, sign);
        free(key), free(sign);
        
        CURL *curl;
        if(reqUrl && (curl = curl_easy_init())) {
            memStream answerData; memStreamInit(&answerData);
            
            char* header;
            asprintf(&header, "Content-Type: application/octet-stream\r\n "
                     "Content-Length: %lu\r\n"
                     "accept-ranges: bytes", fileData.size);
            
            struct curl_slist *slist = curl_slist_append(NULL, header);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
            curl_easy_setopt(curl, CURLOPT_READDATA, &fileData);
            curl_easy_setopt(curl, CURLOPT_READFUNCTION, memStreamRead);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fileData.size);
            
            err = drbOAuthCurlPerform(cli, curl, reqUrl, DRB_HTTP_POST2,
                                      &answerData, answer ? memStreamWrite:NULL,
                                      NULL, timeout);
            
            free(header);
            curl_slist_free_all(slist);
            curl_easy_cleanup(curl);
            free(fileData.data);
            
            if (answer)
                *answer = answerData.data;
        } else
            err = DRBERR_MALLOC;
        
        free(reqUrl);
    } else
        err = DRBERR_MALLOC;
    
    return err;
}

