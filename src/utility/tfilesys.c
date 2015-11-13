
#include "stdafx.h"

#include "tfilesys.h"

#ifdef _WINDOWS
#include <io.h>
#include <direct.h>
#include <windows.h>
#else
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#endif

#include <ctype.h>
#include "tstring.h"

#include "talloc.h"

#include "debugging/tdebug.h"

#define FILE_PATH_BUFFER_SIZE 256
static char s_filePathBuffer[FILE_PATH_BUFFER_SIZE + 1];

static unsigned char FilterMatch(const char *_filename, const char *_filter)
{
	const char *_filterptr = 0;

	while(*_filename && *_filter) {
		if(tolower(*_filename)== tolower(*_filter) || *_filter == '?') {
			_filename++;
			_filter++;
		} else if (*_filter == '*') {
			if(!*(++_filter)) return 1;
			_filterptr = _filter;
			_filename++;
		} else if(_filterptr) {
			_filter = _filterptr;
			_filename++;
		} else {
			return 0;
		}
	}

	return !*_filename && !*_filter;
}

/*static unsigned char FilterMatch(const char *_filename, const char *_filter)
  {
  while(*_filename && tolower(*_filename)== tolower(*_filter)) {
  _filename++;
  _filter++;
  }

  if(tolower(*_filename)== tolower(*_filter))
  return 1;

  switch(*_filter++) {
  case '*':
  while(*_filename) {
  _filename++;
  if(FilterMatch(_filename, _filter))
  return 1;
  }
  return 0;

  case '?':
  if(!*_filename)
  return 0;
  _filename++;
  return FilterMatch(_filename, _filter);
  default:
  return 0;
  }
  }*/

size_t TFileSysListDirectory(char **output, const char *_dir, const char *_filter, unsigned char _fullFilename)
{
	size_t idx = 0;
#ifdef _WINDOWS
	unsigned int dirlen = 0;
	long fileindex = -1;
	struct _finddata_t thisfile;
	char *dir;
#else
	DIR *dir;
	struct dirent *entry;
#endif

	if(!_filter || !(*_filter)) _filter = "*";
	if(!_dir || !(*_dir))       _dir = ".";

	// Now add on all files found locally
#ifdef _WINDOWS
	dirlen = strlen(_dir);
	dir = TFileSysConcatPaths(_dir, _filter, NULL);
	if(!dir) return 0;

	fileindex = _findfirst(dir, &thisfile);
	free(dir);

	if(fileindex != -1) {
		do {
			if(!(thisfile.attrib & _A_SUBDIR)) {
				char *newname = 0;
				int len = strlen(thisfile.name) + 1;
				if(_fullFilename) len += dirlen + 1;

				newname = (char *) malloc(len);
				if(_fullFilename)
					sprintf(newname, "%s/%s", _dir, thisfile.name);
				else
					sprintf(newname, "%s", thisfile.name);

				output = TRAlloc(output,sizeof(char *) * (idx + 1));
				output[idx] = newname;
				idx++;
			}
		} while(!_findnext(fileindex, &thisfile));
	}
#else
	dir = opendir(_dir);
	if(!dir) return 0;
	for(;(entry = readdir(dir))!= NULL;) {
		if(FilterMatch(entry->d_name, _filter)) {
			char fullname[strlen(_dir) + strlen(entry->d_name) + 2];
			sprintf(fullname, "%s%s%s", _dir, _dir[0] ? "/" : "", entry->d_name);
			if(!TFileSysIsDirectory(fullname)) {
				output = TRAlloc(output,sizeof(char *) * (idx + 1));
				output[idx] = strdup(_fullFilename ? fullname : entry->d_name);
				idx++;
			}
		}
	}
	closedir(dir);
#endif

	return idx;
}

size_t TFileSysListSubDirectoryNames(char **output, const char *_dir)
{
	size_t idx = 0;

#ifdef _WINDOWS
	struct _finddata_t thisfile;
	char *dir = TFileSysConcatPaths(_dir, "*", NULL);
	long fileindex = _findfirst(dir, &thisfile);

	if(fileindex != -1) {
		while(!_findnext(fileindex, &thisfile)) {
			if(thisfile.name[0] != '.' && thisfile.attrib & _A_SUBDIR) {
				char *newname = TStringCopy(thisfile.name);

				output = TRAlloc(output,sizeof(char *) * (idx + 1));
				output[idx] = newname;
				idx++;
			}
		} 
	}

	TFree(dir);
#else
	DIR *dir = opendir(_dir);
	struct dirent *entry;

	if(!dir) return 0;
	for(;(entry = readdir(dir))!= NULL;) {
		if(entry->d_name[0] == '.')
			continue;

		char fullname[strlen(_dir) + strlen(entry->d_name) + 2];
		sprintf(fullname, "%s%s%s", _dir, _dir[0] ? "/" : "", entry->d_name);

		if(TFileSysIsDirectory(fullname)) {
			output = TRAlloc(output,sizeof(char *) * (idx + 1));
			output[idx] = strdup(fullname);
			idx++;
		}
	}
	closedir(dir);
#endif
	return idx;
}

unsigned char TFileSysFileExists(const char *_fullPath)
{
	FILE *f = fopen(TFileSysFindCaseInsensitive(_fullPath), "r");
	if(f) {
		fclose(f);
		return 1;
	}

	return 0;
}

static inline void getParent(char *buf)
{
	char *finalSlash = strrchr(buf,'/');
	if(finalSlash) {
		if(finalSlash[1] == '\x0') {
			char *tmp = finalSlash;
			*finalSlash = '\x0';
			finalSlash = strrchr(buf,'/');
			if(!finalSlash) {
				*tmp = '/';
				return;
			}
		}
		*(finalSlash+1)= '\x0';
	}
}

char *TFileSysConcatPathsFetch(const char *(*func)(void *), void *data)
{
	const char *component;
	char *buffer;
	size_t size;

	if (!func) return 0;

	buffer = TStringCopy(func(data));
	if (!buffer) return 0;

	size = strlen(buffer) + 1;
	component = func(data);

	while (component) {
		if (!strcmp(component, "..")) {
			size_t osize = size;

			getParent(buffer);
			size = strlen(buffer) + 1;
			if (osize != size) buffer = (char *)TRAlloc(buffer, size);
		}
		else if (strcmp(component, ".")) {
			size_t clen = strlen(component);
			if (clen != 0) {
				size_t olen = size - 1;
				unsigned char needsep = buffer[olen - 1] != '/';
				size += clen;
				if (needsep) size += 1;

				buffer = (char *)TRAlloc(buffer, size);
				snprintf(buffer + olen, size - olen, needsep ? "/%s" : "%s", component);
			}
		}

		component = func(data);
	}

	return buffer;
}

char *TFileSysConcatPathsArr(const char **paths, size_t size)
{
	const char *component;
	char *buffer;
	size_t bufferlen, i = 1;

	if(!paths || !size) return 0;

	buffer = TStringCopy(paths[0]);

	if (i < size && !strcmp(paths[i],"..")) {
		do {
			getParent(buffer);
			i++;
		} while(i < size && !strcmp(paths[i],".."));

		bufferlen = strlen(buffer) + 1;
		buffer = (char *)TRAlloc(buffer, bufferlen);
	} else {
		bufferlen = strlen(buffer) + 1;
	}

	for(; i < size;)
	{
		component = paths[i++];
		if(i < size) {
			if(!strcmp(paths[i],"..")) {
				i++;
				continue;
			}
		}

		if(strcmp(component,".")) {
			unsigned char needsep = bufferlen > 1 && buffer[bufferlen-2] != '/';
			size_t oldlen = bufferlen - 1;
			bufferlen += strlen(component);
			if(needsep) bufferlen += 1;

			buffer =(char *)TRAlloc(buffer, bufferlen);
			snprintf(buffer + oldlen,bufferlen-oldlen,needsep ? "/%s" : "%s",component);
		}
	}

	return buffer;
}

char *TFileSysConcatPaths(const char *_firstComponent, ...)
{
	va_list components;
	const char *component;
	char *buffer, *returnBuffer;
	unsigned int bufferlen = 0;

	buffer = TStringCopy(_firstComponent);
	bufferlen = strlen(buffer) + 1;

	va_start(components, _firstComponent);
	while((component = va_arg(components, const char *)))
	{
		if(!strcmp(component,"..")) {
			getParent(buffer);
			bufferlen = strlen(buffer) + 1;

		} else if(strcmp(component,".")) {
			bufferlen += strlen(component) + 1;
			buffer =(char *)realloc(buffer, bufferlen);
			if(buffer[0]) strcat(buffer, "/");
			strcat(buffer, component);
		}
	}
	va_end(components);

	returnBuffer = TStringCopy(buffer);
	TFree(buffer);
	return returnBuffer;
}

char *TFileSysConcatPathsExt(const char *_firstComponent, ...)
{
	va_list components;
	const char *component, *peek;
	char *buffer, *returnBuffer;
	unsigned int bufferlen = 0;

	buffer = TStringCopy(_firstComponent);
	bufferlen = strlen(buffer) + 1;

	va_start(components, _firstComponent);
	component = va_arg(components, const char *);
	while (component)
	{
		peek = va_arg(components, const char *);

		if (!strcmp(component, "..")) {
			getParent(buffer);
			bufferlen = strlen(buffer) + 1;

		}
		else if (strcmp(component, ".")) {
			bufferlen += strlen(component) + 1;
			buffer = (char *)realloc(buffer, bufferlen);
			if (buffer[0]) strcat(buffer, peek ? "/" : ".");
			strcat(buffer, component);
		}
		component = peek;
	}
	va_end(components);

	returnBuffer = TStringCopy(buffer);
	TFree(buffer);
	return returnBuffer;
}

char *TFileSysGetParent(const char *_fullFilePath)
{
	strncpy(s_filePathBuffer, _fullFilePath, FILE_PATH_BUFFER_SIZE);

	getParent(s_filePathBuffer);

	return s_filePathBuffer;
}

char *TFileSysGetDirectory(const char *_fullFilePath)
{
	char *finalSlash = 0;
	strncpy(s_filePathBuffer, _fullFilePath, FILE_PATH_BUFFER_SIZE);

	finalSlash = strrchr(s_filePathBuffer,'/');
	if(finalSlash) {
		*(finalSlash+1)= '\x0';
		return s_filePathBuffer;
	}

	return 0;
}

char *TFileSysGetDirname(const char *_fullFilePath)
{
	char *cpy;
	const char *start, *end;

	if(!_fullFilePath) return 0;

	end = strrchr(_fullFilePath, '/');
	if(!end) return 0;

	if(end == _fullFilePath) {
		start = 0;
	} else {
		const char *cur = _fullFilePath;
		start = _fullFilePath;
		while ((cur = strchr(cur,'/')) != end) { cur += 1; start = cur; }
	}

	if(!(end-start)) return 0;
	cpy = (char *) malloc(sizeof(char) * (end-start+1));
	if(cpy) memcpy(cpy, start, end-start);
	cpy[end-start] = 0;

	return cpy;
}

const char *TFileSysGetFilename(const char *_fullFilePath)
{
	const char *result = strrchr(_fullFilePath, '/');
	return result ? result + 1 : result;
}

const char *TFileSysGetExtension(const char *_fullFilePath)
{
	const char *result = strrchr(_fullFilePath, '.');
	return result ? result + 1 : result;
}

char *TFileSysRemoveExtension(const char *_fullFileName)
{
	char *dot = 0;
	strcpy(s_filePathBuffer, _fullFileName);

	dot = strrchr(s_filePathBuffer, '.');
	if(dot)				*dot = '\x0';
	return s_filePathBuffer;
}

char **TFileSysSplitPath(const char *path)
{
	size_t len = strlen(path), split = 1, size;
	if (path[len] == '/') split = 2;

	return TStringRSplit(path, "/",&size,split);
}

char **TFileSysSplitPathFull(const char *path,size_t *size)
{
	return TStringSplit(path, "/",size,0);
}

unsigned char TFileSysFilesIdentical(const char *_name1, const char *_name2)
{
	FILE *in1 = fopen(_name1, "r");
	FILE *in2 = fopen(_name2, "r");
	unsigned char rv = 1;
	unsigned char exitNow = 0;

	if(!in1 || !in2) {
		rv = 0;
		exitNow = 1;
	}

	while(!exitNow && !feof(in1)&& !feof(in2)) {
		int a = fgetc(in1);
		int b = fgetc(in2);
		if(a != b) {
			rv = 0;
			exitNow = 1;
			break;
		}
	}

	if(!exitNow && feof(in1)!= feof(in2))
		rv = 0;

	if(in1)fclose(in1);
	if(in2)fclose(in2);

	return rv;
}

TSize TFileSysGetFileSize(const char *path)
{
#ifdef _WINDOWS
	WIN32_FILE_ATTRIBUTE_DATA data;
	if(GetFileAttributesEx(path, GetFileExInfoStandard, &data)) {
		return data.nFileSizeLow;
	}
#else
	struct stat s;
	int rc = stat(path, &s);
	if(!rc)
		return s.st_size;
#endif
	return 0;
}

unsigned char TFileSysCreateDirectory(const char *_directory)
{
#ifdef _WINDOWS
	int result = _mkdir(_directory);
	if(result == 0)									return 0;	// Directory was created
	if(result == -1 && errno == 17 /* EEXIST */)	return 0;	// Directory already exists
	return 1;
#else
	if(access(_directory, W_OK|X_OK|R_OK)== 0)
		return 0;
	return mkdir(_directory, 0755);
#endif
}

unsigned char TFileSysCreateDirectoryRecursively(const char *_directory)
{
	const char *p;
	char *buffer;
	unsigned char error = 0;
	char match;

	if(!strlen(_directory)) return 0;
	match = strchr(_directory, '/') ? '/' : '\\';

	buffer = (char *) TAlloc(strlen(_directory) + 1);
	if(!buffer) return 1;

	p = _directory;
	if(*p == match) p++;

	p = strchr(p, match);
	while(p && !error) {
		memcpy(buffer, _directory, p - _directory + 1);
		buffer[ p-_directory + 1 ] = '\0';
		if (!TFileSysIsDirectory(buffer)) error = TFileSysCreateDirectory(buffer);
		p = strchr(p+1, match);
	}

	TFree(buffer);

	return error ? 1 : TFileSysCreateDirectory(_directory);
}

void TFileSysDelete(const char *_filename)
{
#ifdef _WINDOWS
	DeleteFile(_filename);
#else
	unlink(_filename);
#endif
}

unsigned char TFileSysIsDirectory(const char *_fullPath)
{
#ifdef _WINDOWS
	unsigned long dwAttrib = GetFileAttributes(_fullPath);
	return (dwAttrib != -1 && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat s;
	int rc = stat(_fullPath, &s);
	if(rc != 0) return 0;
	return(s.st_mode & S_IFDIR);
#endif
}

const char *TFileSysFindCaseInsensitive(const char *_fullPath)
{
#ifndef _LINUX
	return _fullPath;
#else
	static char retval[PATH_MAX];
	char *dir = 0, *file = 0;
	const char *res;
	char **files = 0;
	size_t numfiles = 0;

	if(!_fullPath) return 0;

	// Make our own copy of the result, since GetDirectoryPart
	// and GetFilenamePart use the same variable for temp
	// storage.
	if((dir = TFileSysGetDirectory(_fullPath))!= NULL) dir = strdup(dir);

	// No directory provided. Assume working directory.
	if(!dir) file = strdup(_fullPath);
	else {
		dir[strlen(dir) - 1] = '\0';
		file = strdup(TFileSysGetFilename(_fullPath));
	}
	numfiles = TFileSysListDirectory(files, dir, file, 0);

	free(dir); free(file);
	dir = file = 0;

	// We shouldn't have found more than one match.
	if(numfiles > 1) {
		size_t i = 0;
		for(; i < numfiles; ++i) free(files[i]);
		free(files);
		return 0;
	}

	// No results, so maybe the file does not exist.
	if(numfiles == 0) return _fullPath;

	// Copy the corrected path back, and prepare to return it.
	res = (const char *) files[0];
	memset(retval, 0, sizeof(retval));
	TAssert(strlen(res)< PATH_MAX);
	strcpy(retval, res);

	free(files[0]);
	free(files);

	// Negate the possibility of a memory access violation.
	// This way, we can simply strcpy the result inline without
	// worrying about a buffer overflow.
	TAssert(strlen(retval)== strlen(_fullPath));

	return retval;
#endif
}
