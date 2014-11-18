/*!
 * \file    memStream.h
 * \brief   Provide functions to read/write stream from/to memory.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#define _GNU_SOURCE

#define MEM_BUFFER_SIZE 1024

#include <stdlib.h>
#include <string.h>
#include "memStream.h"


void* memRealloc(void* ptr, size_t size) {
    char* newPtr = NULL;
    if (size > 0 && (newPtr = realloc(ptr, size)) == NULL)
        free(ptr), ptr = NULL;
    return newPtr;
}

void memStreamInit(memStream *stream) {
    memset(stream, 0, sizeof(memStream));
}

void memStreamCleanup(memStream *stream) {
    free(stream->data);
    memStreamInit(stream);
}

size_t memStreamWrite(const void *ptr, size_t size, size_t count, memStream *stream) {
    size_t realSize = size * count;
    if((stream->data = memRealloc(stream->data, stream->size + realSize + 1)) != NULL){
        size_t offset = stream->cursor > stream->size ? stream->size : stream->cursor;
        memcpy(stream->data + offset, ptr, realSize);
        stream->cursor = offset + realSize;
        if (stream->cursor > stream->size) stream->size = stream->cursor;
        stream->data[stream->size] = '\0';
        return realSize;
    } else
        return 0;
}

size_t memStreamRead(void *ptr, size_t size, size_t count, memStream *stream) {
    
    size_t realSize;
    if (stream->cursor <= stream->size) {
        realSize = size * count;
        if(stream->cursor + realSize > stream->size)
            realSize = stream->size - stream->cursor;
        if (realSize) {
            memcpy((void*)ptr, stream->data + stream->cursor, realSize);
            stream->cursor += realSize;
        }
    } else
        realSize = 0;
    
    return realSize;
}

/*
 * @ any code reviewer (like myself): dont replace the code with a memStreamPipe
 * call, code is optimized here to reduce memory reallocation (realloc calls).
 */
bool memStreamLoad(memStream* stream, void* data, size_t (*xread)(void *, size_t , size_t , void*)) {
    char buffer[MEM_BUFFER_SIZE];
    size_t len, allocSize = 0;
    
    while ((len = xread(buffer, sizeof(char), MEM_BUFFER_SIZE, data)) > 0) {
        stream->size += len;
        if (stream->size > allocSize) {
            allocSize = stream->size * 2;
            if ((stream->data = memRealloc(stream->data, allocSize)) == NULL)
                break;
        }
        memcpy(stream->data + stream->size - len, buffer, len);
    }
    // is memory still allocated and then shrinked with success ?
    return stream->data && (stream->data = memRealloc(stream->data, stream->size)) != NULL;
}

bool memStreamPipe(void* in,  size_t (*xread)(void*, size_t, size_t, void*),
                   void* out, size_t (*xwrite)(void*, size_t, size_t, void*)) {
    char buffer[MEM_BUFFER_SIZE];
    size_t rLen, wLen;
    while ((rLen = xread(buffer, sizeof(char), MEM_BUFFER_SIZE, in)) > 0) {
        do { // write cached memory
            rLen -= (wLen = xwrite(buffer, 1, rLen, out));
        } while (wLen);
        
        if (rLen)         // flush fail when there's still memory to read or
            return false; // on write overflow (wLen > rLen -> xwrite bug!)
    }
    return true;
}


void memStreamRewind(memStream* stream) {
    stream->cursor = 0;
}

int memStreamSeek(memStream* stream, long int offset, int origin) {
    
    size_t newCursor = 0;
    
    switch (origin) {
        case SEEK_SET: newCursor = offset;
            break;
        case SEEK_CUR: newCursor = stream->cursor + offset;
            break;
        case SEEK_END: newCursor = stream->size + offset;
            break;
    }
    
    if (offset > 0 || newCursor < stream->cursor) {
        stream->cursor = newCursor;
        return 0;
    } else
        return -1;
}



