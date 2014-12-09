/*!
 * \file    dropboxUtils.h
 * \brief   Common function library for dropbox library.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#ifndef DROPBOX_UTILS_H
#define DROPBOX_UTILS_H

char* drbStrDup(const char*);
char* drbGetHeaderFieldContent(const char* field, char* header);

#endif /* DROPBOX_UTILS_H */
