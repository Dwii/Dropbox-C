/*!
 * \file    memStream.h
 * \brief   Provide functions to read/write stream from/to memory.
 * \author  Adrien Python
 * \version 1.0
 * \date    29.10.2013
 */

#ifndef MEM_STREAM_H
#define MEM_STREAM_H

#include <stdbool.h>
#include <stdio.h>

/*!
 * \struct  memStream
 * \breif   Memory space to read/write with memRead/memWrite.
 */
typedef struct {
    char* data;    /*< where the data memory start */
    size_t size;   /*< data memory size */
    size_t cursor; /*< where memRead is currently reading data. */
} memStream;

/*!
 * \brief   As realloc but free unreallocated memory on failure.
 * \param   ptr    pointer on the memory to realloc
 * \param   size   needed memory size
 * \return  pointer to the reallocated memory
 */
void* memRealloc(void* ptr, size_t size);

/*!
 * \brief   Initialize a memory stream before its use.
 * \param   stream   memory stream to initialize
 * \return  void
 */
void memStreamInit(memStream *stream);

/*!
 * \brief   Release allocated memory in a memory stream.
 * \param   stream   memory stream to initialize
 * \return  void
 */
void memStreamCleanup(memStream *stream);

/*!
 * As fwrite, but for a memory stream.
 */
size_t memStreamWrite(const void *ptr, size_t size, size_t count, memStream *mem);

/*!
 * As fread, but for a memory stream.
 */
size_t memStreamRead(void *ptr, size_t size, size_t count, memStream *mem);

/*!
 * \brief   Load any kind of data into a memory stream.
 * \note    Use this instead of memStreamPipe as memory allocation is optimized.
 * \param   stream    stream to load with data
 * \param   data      data to load in stream
 * \param   readFct   function to read data content
 * \return  indicates whether the data were loaded successfully or not.
 */
bool memStreamLoad(memStream* stream, void* data,
                   size_t (*xread)(void *, size_t , size_t , void*));

/*!
 * \breif   Transfert any kind of stream from a one to another.
 *
 * Try to avoid it when you know which kind of stream you deal with, as there
 * might be a more efficent way to deal with them:
 *   -# memStreamPipe(mem, memStreamRead, file, fwrite):
 *      -> use fwrite directly: fwrite(mem->data, 1, mem->size, file)
 *   -# memStreamPipe(aMem, memStreamRead, bMem, memStreamWrite)
 *      -> just copy the memory yourself: malloc + memcpy
 *   -# memStreamPipe(aFile, fread, bFile, fwrite)
 *      -> file copy, valid but there's might be better implemtation for this
 *   -# memStreamPipe(file, fread, mem, memStreamWrite)
 *      -> use memStreamLoad for a better memory allocation
 *
 * \param   in       input stream
 * \param   xread    function to read the input stream
 * \param   out      ouput stream
 * \param   xwrite   function to write the output stream
 * \return  indicates whether the data were 'piped' successfully or not.
 */
bool memStreamPipe(void* in,  size_t (*xread)(void*, size_t ,size_t ,void*),
                   void* out, size_t (*xwrite)(void*, size_t ,size_t ,void*));


/*!
 * \brief   Sets the stream position to the beginning his data.
 * \param   stream    stream to rewind
 * \return  void
 */
void memStreamRewind(memStream* stream);

/*!
 * \brief   Sets the position indicator of the stream to a new position.
 * \param   stream    stream to handle
 * \param   offset    number of bytes to offset from origin
 * \param   origin    offset reference (SEEK_SET, SEEK_CUR, SEEK_END)
 * \return  if successful, returns zero. Otherwise, returns non-zero value.
 */
int memStreamSeek(memStream* stream, long int offset, int origin);


#endif /* MEM_STREAM_H */