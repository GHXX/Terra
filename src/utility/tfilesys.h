
#ifndef __included_terra_filesys_h
#define __included_terra_filesys_h

TSize TFileSysListDirectory(char **output, const char *_dir, const char *_filter, unsigned char fullFilename);
TSize TFileSysListSubDirectoryNames(char **output, const char *_dir);

unsigned char TFileSysIsDirectory(const char *_fullPath);
unsigned char TFileSysFileExists(const char *_fullPath);
unsigned char TFileSysFilesIdentical(const char *_name1, const char *_name2);

char *TFileSysConcatPathsFetch(const char *(*func)(void *), void *data);
char *TFileSysConcatPathsArr(const char **paths, TSize size);
char *TFileSysConcatPaths(const char *_firstComponent, ...);

char *TFileSysConcatPathsExt(const char *_firstComponent, ...);

char *TFileSysGetParent(const char *_fullFilePath);
char *TFileSysGetDirectory(const char *_fullFilePath);
char *TFileSysGetDirname(const char *_fullFilePath);
const char *TFileSysGetFilename(const char *_fullFilePath);
const char *TFileSysGetExtension(const char *_fileFilePath);
char *TFileSysRemoveExtension(const char *_fullFileName);

char **TFileSysSplitPath(const char *path);
char **TFileSysSplitPathFull(const char *path,TSize *size);

TSize TFileSysGetFileSize(const char *path);

unsigned char TFileSysCreateDirectory(const char *_directory);
unsigned char TFileSysCreateDirectoryRecursively (const char *_directory);
void TFileSysDelete(const char *_filename);

const char *TFileSysFindCaseInsensitive(const char *_fullPath);

#endif
