
#ifndef __included_terra_stream_h
#define __included_terra_stream_h

/**
* Terra Stream
*
*   The purpose of this file is to provide a common
*   reading and writing interface.
*
*/

typedef struct _TStream TStream;
typedef TPtr TStreamContent;

typedef struct _TStreamOps {
	TSize(*size) (TStreamContent *content);

	int(*seek) (TStreamContent *content, TLInt offset, TInt8 origin);
	TLInt(*tell) (TStreamContent *content);

	unsigned char(*eof) (TStreamContent *content);

	TSize(*read) (TStreamContent *content, TPtr buffer, TSize size);
	TSize(*write) (TStreamContent *content, TCPtr buffer, TSize size);

	int(*close) (TStreamContent *content);
} TStreamOps;

/**
* Creates and returns a stream context for a new buffer
*
* @return                    A stream context.
*
*/
TStream *TStreamNew(void);

/**
* Opens a file, creates a stream context for it and returns it
*
* @param filepath            The path to the file.
* @param mode                The file opening mode.
*
* @return                    A stream context.
*
*/
TStream *TStreamFromFile(const char *filepath, const char *mode);

/**
* Creates and returns a stream context for a file
*
* @param file                A file handle.
* @param autoclose           specify whether the file should be closed when
*                            the stream context is being freed.
*
* @return                    A stream context.
*
*/
TStream *TStreamFromFilePointer(FILE *file, unsigned char autoclose);

/**
* Creates and returns a stream context for a buffer
*
* @param buffer              A buffer.
* @param size                The buffer size.
* @param autofree            specify whether the buffer should be freed when
*                            the stream context is being freed.
*
* @return                    A stream context.
*
*/
TStream *TStreamFromMem(unsigned char **buffer, TSize size, unsigned char autofree);

/**
* Creates and returns a stream context for a const buffer
*
* @param buffer              A buffer.
* @param size                The buffer size.
*
* @return                    A stream context.
*
*/
TStream *TStreamFromConstMem(const unsigned char *buffer, TSize size);

/**
* Creates and returns a stream context for an unspecified content
*
* @param content             A user specified content.
* @param ops                 A set of operations.
*
* @return                    A stream context.
*
*/
TStream *TStreamFromContent(TStreamContent content, TStreamOps *ops, TFreeFunc clean);

/**
* Frees a stream context
*
* @param context             A stream context.
*
*/
void TStreamFree(TStream *context);

/**
* Change the set of operations for a stream context.
*
* @param context             A stream context.
* @param ops                 A set of operations.
*
*/
void TStreamSetOps(TStream *context, TStreamOps *ops);

/**
* Get the remaining size for the stream context.
*
* @param context             A stream context.
*
* @return                    The remaining size.
*
*/
TSize TStreamSize(TStream *context);

/**
* Sets the cursor position for the stream context.
*
* @param context             A stream context.
* @param offset              The offset from the origin.
* @param origin              The starting position(SEEK_CUR, SEEK_SET, SEEK_END).
*
* @return                    An error code.
*
*/
int TStreamSeek(TStream *context, TLInt offset, TInt8 origin);

/**
* Get the current cursor position for the stream context.
*
* @param context             A stream context.
*
* @return                    the stream position.
*
*/
TLInt TStreamTell(TStream *context);

/**
* Verify whether the end of content has been reached.
*
* @param context             A stream context.
*
* @return                    0 for false, true otherwise.
*
*/
int TStreamEOF(TStream *context);

/**
* Read 8 bits from the context
*
* @param context             A stream context.
*
* @return                    A Byte worth of data.
*
*/
TUInt8 TStreamRead8(TStream *context);

/**
* Read 16 bits from the context
*
* @param context             A stream context.
*
* @return                    2 Bytes worth of data.
*
*/
TUInt16 TStreamRead16(TStream *context);

/**
* Read 32 bits from the context
*
* @param context             A stream context.
*
* @return                    4 Bytes worth of data.
*
*/
TUInt32 TStreamRead32(TStream *context);

/**
* Read 64 bits from the context
*
* @param context             A stream context.
*
* @return                    8 Bytes worth of data.
*
*/
TUInt64 TStreamRead64(TStream *context);

/**
* Fill a user specified buffer with data
*
* @param context             A stream context.
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
* @param context             A stream context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite8(TStream *context, TUInt8 data);

/**
* Writes 16 bits to the context buffer
*
* @param context             A stream context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite16(TStream *context, TUInt16 data);

/**
* Writes 32 bits to the context buffer
*
* @param context             A stream context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite32(TStream *context, TUInt32 data);

/**
* Writes 64 bits to the context buffer
*
* @param context             A stream context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TStreamWrite64(TStream *context, TUInt64 data);

/**
* Writes a block of data to the context buffer
*
* @param context             A stream context.
* @param buffer              A user specified buffer.
* @param size                The buffer size.
*
* @return                    The amount written.
*
*/
int TStreamWriteBlock(TStream *context, const unsigned char *buffer, TSize size);

#endif
