
#ifndef __included_terra_read_write_h
#define __included_terra_read_write_h

/**
* Terra Read Write
*
*   The purpose of this file is to provide a common
*   reading and writing interface.
*
*/

typedef struct _TRW TRW;

typedef struct _TRWOps {
	TSize(*size) (TRW *context);

	int(*seek) (TRW *context, TSize offset, int origin);
	int(*tell) (TRW *context);

	unsigned char(*eof) (TRW *context);

	TSize(*read) (TRW *context, TPtr buffer, TSize size);
	TSize(*write) (TRW *context, TCPtr buffer, TSize size);

    int(*close) (TRW *context);
} TRWOps;

typedef TPtr TRWContent;

/**
* Opens a file, creates a read write context for it and returns it
*
* @param filepath            The path to the file.
* @param mode                The file opening mode.
*
* @return                    A read write context.
*
*/
TRW *TRWFromFile(const char *filepath, const char *mode);

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
TRW *TRWFromFilePointer(FILE *file, unsigned char autoclose);

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
TRW *TRWFromMem(unsigned char *buffer, TSize size, unsigned char autofree);

/**
* Creates and returns a read write context for a const buffer
*
* @param buffer              A buffer.
* @param size                The buffer size.
*
* @return                    A read write context.
*
*/
TRW *TRWFromConstMem(const unsigned char *buffer, TSize size);

/**
* Creates and returns a read write context for an unspecified content
*
* @param content             A user specified content.
* @param ops                 A set of operations.
*
* @return                    A read write context.
*
*/
TRW *TRWFromContent(TRWContent content, const TRWOps ops);

/**
* Frees a read write context
*
* @param context             A read write context.
*
*/
void TRWFree(TRW *context);

/**
* Change the set of operations for a read write context.
*
* @param context             A read write context.
* @param ops                 A set of operations.
*
*/
void TRWSetOps(TRW *context, const TRWOps ops);

/**
* Get the remaining size for the read write context.
*
* @param context             A read write context.
*
* @return                    The remaining size.
*
*/
TSize TRWSize(TRW *context);

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
int TRWSeek(TRW *context, TSize offset, int origin);

/**
* Get the current cursor position for the read write context.
*
* @param context             A read write context.
*
* @return                    An error code.
*
*/
int TRWTell(TRW *context);

/**
* Verify whether the end of content has been reached.
*
* @param context             A read write context.
*
* @return                    0 for false, true otherwise.
*
*/
unsigned char TRWEOF(TRW *context);

/**
* Read 8 bits from the context
*
* @param context             A read write context.
*
* @return                    A Byte worth of data.
*
*/
unsigned char TRWRead8(TRW *context);

/**
* Read 16 bits from the context
*
* @param context             A read write context.
*
* @return                    2 Bytes worth of data.
*
*/
unsigned short TRWRead16(TRW *context);

/**
* Read 32 bits from the context
*
* @param context             A read write context.
*
* @return                    4 Bytes worth of data.
*
*/
unsigned int TRWRead32(TRW *context);

/**
* Read 64 bits from the context
*
* @param context             A read write context.
*
* @return                    8 Bytes worth of data.
*
*/
unsigned long long TRWRead64(TRW *context);

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
TSize TRWReadBlock(TRW *context, unsigned char *buffer, TSize size);

/**
* Writes 8 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TRWWrite8(TRW *context, unsigned char data);

/**
* Writes 16 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TRWWrite16(TRW *context, unsigned short data);

/**
* Writes 32 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TRWWrite32(TRW *context, unsigned int data);

/**
* Writes 64 bits to the context buffer
*
* @param context             A read write context.
* @param data                A user specified data.
*
* @return                    The amount written.
*
*/
int TRWWrite64(TRW *context, unsigned long long data);

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
int TRWWriteBlock(TRW *context, const unsigned char *buffer, TSize size);

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
int TRWWriteString(TRW *context, const char *buffer, TSize size);

/**
* Writes a variable list to the context buffer
*
* @param context             A read write context.
* @param format              The format used to write.
*
*/
void TRWWrite(TRW *context, const char *format,...);

/**
* Writes a variable list to the context buffer
*
* @param context             A read write context.
* @param format              The format used to write.
*
*/
void TRWWriteV(TRW *context, const char *format, va_list list);

#endif
