
#ifndef __included_terra_utility_filesys_h
#define __included_terra_utility_filesys_h

#include "tstring.h"

TSize TFileSysListDirectory(char **results, const char *_dir, const char *_filter, unsigned char fullFilename);
TSize TFileSysListSubDirectoryNames(char **results, const char *_dir);

unsigned char TFileSysIsDirectory(const char *_fullPath);
unsigned char TFileSysFileExists(const char *_fullPath);
unsigned char TFileSysFilesIdentical(const char *_name1, const char *_name2);

unsigned char TFileSysIsFullPath(const char *path);
unsigned char TFileSysIsWindowsPath(const char *path);

char *TFileSysGetWorkingDirectory(void);

char *TFileSysFixFolderPath(const char *path);
char *TFileSysFixFilePath(const char *path);

char *TFileSysConcat(const char *_firstComponent, ...);
char *TFileSysConcatExt(const char *_firstComponent, ...);

char *TFileSysGetParent(char *_fullFilePath);
char *TFileSysGetDirectory(const char *_fullFilePath);
char *TFileSysGetDirname(const char *_fullFilePath);
const char *TFileSysGetFilename(const char *_fullFilePath);
const char *TFileSysGetExtension(const char *_fileFilePath);
char *TFileSysRemoveExtension(const char *_fullFileName);

char **TFileSysSplitPath(const char *path);
char **TFileSysSplitPathFull(const char *path, TSize *size);

TSize TFileSysGetFileSize(const char *path);
int TFileSysIsReadOnly(FILE *f);

FILE *TFileSysOpen(const char *path, const char *mode);
#define TFileSysClose(f) fclose(f);

unsigned char TFileSysCreateDirectory(const char *path);
unsigned char TFileSysCreateDirectoryRecursively(const char *path);
void TFileSysDelete(const char *path);

char *TFileSysFindCaseInsensitive(const char *_fullPath);

#endif
