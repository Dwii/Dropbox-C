/*!
 * \file    example.c
 * \brief   Usage example of dropbox C library.
 * \author  Adrien Python
 * \version 1.0
 * \date    06.08.2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dropbox.h>
#include <memStream.h>

void displayAccountInfo(drbAccountInfo *info);
void displayMetadata(drbMetadata *meta, char *title);
void displayMetadataList(drbMetadataList* list, char* title);

int main (int argc, char **argv) {
    
    int err;
    void* output;
    
    // Application key and secret. To get them, create an app here:
    // https://www.dropbox.com/developers/apps
    char *c_key    = "your consumer key";  //< consumer key
    char *c_secret = "your consumer secret";  //< consumer secret
    
    // User key and secret. Leave them NULL or set them with your AccessToken.
    char *t_key    = NULL; //< access token key
    char *t_secret = NULL;  //< access token secret
    
    // Global initialisation
    drbInit();
    
    // Create a Dropbox client
    drbClient* cli = drbCreateClient(c_key, c_secret, t_key, t_secret);
    
    // Request a AccessToken if undefined (NULL)
    if(!t_key || !t_secret) {
        drbOAuthToken* reqTok = drbObtainRequestToken(cli);
        
        if (reqTok) {
            char* url = drbBuildAuthorizeUrl(reqTok);
            printf("Please visit %s\nThen press Enter...\n", url);
            free(url);
            fgetc(stdin);
            
            drbOAuthToken* accTok = drbObtainAccessToken(cli);
            
            if (accTok) {
                // This key and secret can replace the NULL value in t_key and
                // t_secret for the next time.
                printf("key:    %s\nsecret: %s\n", accTok->key, accTok->secret);
            } else{
                fprintf(stderr, "Failed to obtain an AccessToken...\n");
                return EXIT_FAILURE;
            }
        } else {
            fprintf(stderr, "Failed to obtain a RequestToken...\n");
            return EXIT_FAILURE;
        }
    }
    
    // Set default arguments to not repeat them on each API call
    drbSetDefault(cli, DRBOPT_ROOT, DRBVAL_ROOT_AUTO, DRBOPT_END);
    
    // Read account Informations
    output = NULL;
    err = drbGetAccountInfo(cli, &output, DRBOPT_END);
    if (err != DRBERR_OK) {
        printf("Account info error (%d): %s\n", err, (char*)output);
        free(output);
    } else {
        drbAccountInfo* info = (drbAccountInfo*)output;
        displayAccountInfo(info);
        drbDestroyAccountInfo(info);
    }
    
    // List root directory files
    output = NULL;
    err = drbGetMetadata(cli, &output,
                         DRBOPT_PATH, "/",
                         DRBOPT_LIST, true,
                         //                     DRBOPT_FILE_LIMIT, 100,
                         DRBOPT_END);
    if (err != DRBERR_OK) {
        printf("Metadata error (%d): %s\n", err, (char*)output);
        free(output);
    } else {
        drbMetadata* meta = (drbMetadata*)output;
        displayMetadata(meta, "Metadata");
        drbDestroyMetadata(meta, true);
    }
    
    // Try to download a file named hello.txt
    FILE *file = fopen("/tmp/hello.txt", "w"); // Write it in this file
    output = NULL;
    err = drbGetFile(cli, &output,
                     DRBOPT_PATH, "/hello.txt",
                     DRBOPT_IO_DATA, file,
                     DRBOPT_IO_FUNC, fwrite,
                     DRBOPT_END);
    fclose(file);
    
    if (err != DRBERR_OK) {
        printf("Get File error (%d): %s\n", err, (char*)output);
        free(output);
    } else {
        displayMetadata(output, "Get File Result");
        drbDestroyMetadata(output, true);
    }
    
    
    // Upload this file if it doesnt exists
    if (err == 404) {
        memStream stream; memStreamInit(&stream);
        stream.data = "hello world!";
        stream.size = strlen(stream.data);
        
        err = drbPutFile(cli, NULL,
                         DRBOPT_PATH, "/hello.txt",
                         DRBOPT_IO_DATA, &stream,
                         DRBOPT_IO_FUNC, memStreamRead,
                         DRBOPT_END);
        printf("File upload: %s\n", err ? "Failed" : "Successful");
    }
    
    // Free all client allocated memory
    drbDestroyClient(cli);
    
    // Global Cleanup
    drbCleanup();
    
    return EXIT_SUCCESS;
}


char* strFromBool(bool b) { return b ? "true" : "false"; }

/*!
 * \brief  Display a drbAccountInfo item in stdout.
 * \param  info    account informations to display.
 * \return  void
 */
void displayAccountInfo(drbAccountInfo* info) {
    if(info) {
        printf("---------[ Account info ]---------\n");
        if(info->referralLink)         printf("referralLink: %s\n", info->referralLink);
        if(info->displayName)          printf("displayName:  %s\n", info->displayName);
        if(info->uid)                  printf("uid:          %d\n", *info->uid);
        if(info->country)              printf("country:      %s\n", info->country);
        if(info->email)                printf("email:        %s\n", info->email);
        if(info->quotaInfo.datastores) printf("datastores:   %u\n", *info->quotaInfo.datastores);
        if(info->quotaInfo.shared)     printf("shared:       %u\n", *info->quotaInfo.shared);
        if(info->quotaInfo.quota)      printf("quota:        %u\n", *info->quotaInfo.quota);
        if(info->quotaInfo.normal)     printf("normal:       %u\n", *info->quotaInfo.normal);
    }
}

/*!
 * \brief  Display a drbMetadata item in stdout.
 * \param  meta    metadata to display.
 * \param   title   display the title before the metadata.
 * \return  void
 */
void displayMetadata(drbMetadata* meta, char* title) {
    if (meta) {
        if(title) printf("---------[ %s ]---------\n", title);
        if(meta->hash)        printf("hash:        %s\n", meta->hash);
        if(meta->rev)         printf("rev:         %s\n", meta->rev);
        if(meta->thumbExists) printf("thumbExists: %s\n", strFromBool(*meta->thumbExists));
        if(meta->bytes)       printf("bytes:       %d\n", *meta->bytes);
        if(meta->modified)    printf("modified:    %s\n", meta->modified);
        if(meta->path)        printf("path:        %s\n", meta->path);
        if(meta->isDir)       printf("isDir:       %s\n", strFromBool(*meta->isDir));
        if(meta->icon)        printf("icon:        %s\n", meta->icon);
        if(meta->root)        printf("root:        %s\n", meta->root);
        if(meta->size)        printf("size:        %s\n", meta->size);
        if(meta->clientMtime) printf("clientMtime: %s\n", meta->clientMtime);
        if(meta->isDeleted)   printf("isDeleted:   %s\n", strFromBool(*meta->isDeleted));
        if(meta->mimeType)    printf("mimeType:    %s\n", meta->mimeType);
        if(meta->revision)    printf("revision:    %d\n", *meta->revision);
        if(meta->contents)    displayMetadataList(meta->contents, "Contents");
    }
}

/*!
 * \brief  Display a drbMetadataList item in stdout.
 * \param  list    list to display.
 * \param   title   display the title before the list.
 * \return  void
 */
void displayMetadataList(drbMetadataList* list, char* title) {
    if (list){
        printf("---------[ %s ]---------\n", title);
        for (int i = 0; i < list->size; i++) {
            
            displayMetadata(list->array[i], list->array[i]->path);
        }
    }
}