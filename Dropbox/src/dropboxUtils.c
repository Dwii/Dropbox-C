/*!
 * \file    dropboxUtils.c
 * \brief   Common function library for dropbox library.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#define _GNU_SOURCE

#define BUFFER_SIZE 1024

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <oauth.h>
#include <stdbool.h>
#include "dropboxUtils.h"

/*!
 * \brief   As strdup but return NULL, if str is NULL.
 * \param   str    string to duplicate
 * \return  duplicated string.
 */
char* drbStrDup(const char* str) {
    return str ? strdup(str) : NULL;
}

/*!
 * \brief   Found and return the content of an http header field.
 * \param   field    field name
 * \param   header   header to parse
 * \return  copy of the field content (must be freed by caller)
 */
char* drbGetHeaderFieldContent(const char* field, char* header) {
    char* content = NULL;
    char* fieldLine = strstr(header, field); // find field line
    
    if (fieldLine && (fieldLine == header || *(fieldLine-1) == '\n')) {
        fieldLine += strlen(field) + 1; // Get field content starting point
        
        // Measure the field size
        int length = 0;
        while (fieldLine[length] && fieldLine[length] != '\n' && fieldLine[length] != '\r')
            length++;
        
        // Create a copy of the field content
        if((content = malloc(length + 1)) != NULL) {
            strncpy(content, fieldLine, length);
            content[length] = '\0';
        }
    }
    
    return content;
}
