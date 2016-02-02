
#ifndef __included_terra_read_write_h
#define __included_terra_read_write_h

/**
* Terra Stream
*
*   The purpose of this file is to provide a common
*   reading and writing interface.
*
*/

typedef struct _TStream TStream;

typedef struct _TStreamOps {
	TSize(*size) (TStream *context);

	int(*seek) (TStream *context, TSize offset, int origin);
	int(*tell) (TStream *context);

	unsigned char(*eof) (TStream *context);

	TSize(*read) (TStream *context, TPtr buffer, TSize size);
	TSize(*write) (TStream *context, TCPtr buffer, TSize size);

    int(*close) (TStream *context);
} TStreamOps;

typedef TPtr TStreamContent;

/**
* Opens a file, creates a read write context for it and returns it
*
* @param filepath            The path to the file.
* @param mode                The file opening mode.
*
* @return                    A read write context.
*
*/
TStream *TStreamFromFile(const char *filepath, const char *mode);

/**
* Creates and returns a read write context for a file
*
* @param file                A file handle.
* @param autoclose           specify whether the file should be closed when
*                            the read write context is being freed.
*
* @return                    A read write context.
*
*/
TStream *TStreamFromFilePointer(FILE *file, unsigned char autoclose);

/**
* Creates and returns a read write context for a buffer
*
* @param buffer              A buffer.
* @param size                The buffer size.
* @param autofree            specify whether the buffer should be freed when
*                            the read write context is being freed.
*
* @return                    A read write context.
*
*/
TStream *TStreamFromMem(unsigned char *buffer, TSize size, unsigned char autofree);

/**
* Creates and returns a read write context for a const buffer
*
* @param buffer              A buffer.
* @param size                The buffer size.
*
* @return                    A read write context.
*
*/
TStream *TStreamFromConstMem(const unsigned char *buffer, TSize size);

/**
* Creates and returns a read write context for an unspecified content
*
* @param content             A user specified content.
* @param ops                 A set of operations.
*
* @return                    A read write context.
*
*/
TStream *TStreamFromContent(TStreamContent content, const TStreamOps ops);

/**
* Frees a read write context
*
* @param context             A read write context.
*
*/
void TStreamFree(TStream *context);

/**
* Change the set of operations for a read write context.
*
* @param context             A read write context.
* @param ops                 A set of operations.
*
*/
void TStreamSetOps(TStream *context, const TStreamOps ops);

/**
* Get the remaining size for the read write context.
*
* @param context             A read write context.
*
* @return                    The remaining size.
*
*/
TSize TStreamSize(TStream *context);

/**
* Sets the cursor position for the read write context.
*
* @param context             A read write context.
* @param offset              The offset from the origin.
* @param origin              The starting position(SEEK_CUR, SEEK_SET, SEEK_END).
*
* @return                    An error code.
*
*/
int TStreamSeek(TStream *context, TSize offset, int origin);

/**
* Get the current cursor position for the read write context.
*
* @param context             A read write context.
*
* @return                    An error code.
*
*/
int TStreamTell(TStream *context);

/**
* Verify whether the end of content has been reached.
*
* @param context             A read write context.
*
* @return                    0 for false, true otherwise.
*
*/
unsigned char TStreamEOF(TStream *context);

/**
* Read 8 bits from the context
*
* @param context             A read write context.
*
* @return                    A Byte worth of data.
*
*/
unsigned char TStreamRead8(TStream *context);

/**
* Read 16 bits from the context
*
* @param context             A read write context.
*
* @return                    2 Bytes worth of data.
*
*/
unsigned short TStreamRead16(TStream *context);

/**
* Read 32 bits from the context
*
* @param context             A read write context.
*
* @return                    4 Bytes worth of data.
*
*/
unsigned int TStreamRead32(TStream *context);

/**
* Read 64 bits from the context
*
* @param context             A read write context.
*
* @return                    8 Bytes worth of data.
*
*/
unsigned long long TStreamRead64(TStream *context);

/**
* Fill a user specified buffer with data
*
* @param context             A read write context.
* @param buffer              A user specified buffer.
* @param size                The buffer size.
*
* @return                    The size read.
*
*/
TSize TStreamReadBlock(TStream *context, unsigned char *buffer, TSize size);

/**
* Writes 8 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite8(TStream *context, unsigned char data);

/**
* Writes 16 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite16(TStream *context, unsigned short data);

/**
* Writes 32 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite32(TStream *context, unsigned int data);

/**
* Writes 64 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite64(TStream *context, unsigned long long data);

/**
* Writes a block of data to the context buffer
*
* @param context             A read write context.
* @param buffer              A user specified buffer.
* @param size                The buffer size.
*
* @return                    The amount written.
*
*/
int TStreamWriteBlock(TStream *context, const unsigned char *buffer, TSize size);

/**
* Writes a string to the context buffer
*
* @param context             A read write context.
* @param buffer              The string to be written.
* @param size                The string size (auto-evaluates with 0).
*
* @return                    The amount written.
*
*/
int TStreamWriteString(TStream *context, const char *buffer, TSize size);

#endif
