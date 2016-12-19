
#include "stdafx.h"

#include "tfilesys.h"

#ifdef _WINDOWS
#include <io.h>
#include <direct.h>
#    ifndef getcwd
#        define getcwd _getcwd
#    endif

#    ifdef PLATFORM_X86_64
#		define fseek _fseeki64
#		define ftell _ftelli64
#    endif

#else
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "tencoding.h"

#include "talloc.h"

static unsigned char FilterMatch(const char *_filename, const char *_filter) {
	const char *_filterptr = _filter;

	while (*_filterptr && *_filename) {
		if (*_filterptr == '*') {
			if (!*(++_filterptr)) return 1;
			_filename++;
		} else if (*_filterptr == '?' || tolower(*_filename) == tolower(*_filterptr)) {
			_filename++;
			_filterptr++;
		}
	}

	return !*_filename && !*_filterptr;
}

TSize TFileSysListDirectory(char **results, const char *_dir, const char *_filter, unsigned char fullFilename) {
	TSize size, num;
#ifdef _WINDOWS
	intptr_t fileindex = -1;
	struct _wfinddatai64_t thisfile;
	wchar_t *search;
	char *path;
#else
	DIR *dir;
	struct dirent *entry;
#endif

	if (!results) return 0;
	size = num = 0;
	*results = 0;

	if (!_filter || !(*_filter)) _filter = "*";
	if (!_dir || !(*_dir)) _dir = ".";

	// Now add on all files found locally
#ifdef _WINDOWS
	path = TFileSysConcat(_dir, _filter, 0);
	if (!path) return 0;
	search = TStringToWideChar(path);
	TFree(path);

	fileindex = _wfindfirst64(search, &thisfile);
	TFree(search);

	if (fileindex != -1) {
		do {
			if (!(thisfile.attrib & _A_SUBDIR)) {
				TSize pathSize;
				char *fileName = TStringFromWideChar(thisfile.name);
				if (fullFilename) {
					path = TStringConcatSeparator("/", _dir, fileName, 0);
					TFree(fileName);
				} else {
					path = fileName;
				}

				pathSize = TStringSize(path);
				*results = (char *)TRAlloc(*results, size + pathSize);
				if (!results) return 0;
				memcpy(results + size, path, pathSize);
				TFree(path);

				num++;
				size += pathSize;
			}
		} while (!_wfindnext64(fileindex, &thisfile));
	}
#else
	dir = opendir(_dir);
	if (!dir) return 0;
	for (; (entry = readdir(dir)) != NULL;) {
		if (FilterMatch(entry->d_name, _filter)) {
			TSize pathSize;
			char *fileName = TStringFromWideChar(thisfile.name);
			if (fullFilename) {
				path = TStringConcatSeparator('/', _dir, fileName, 0);
				TFree(fileName);
			} else {
				path = fileName;
			}

			pathSize = TStringSize(path);
			*results = (char *)TRAlloc(*results, size + pathSize);
			if (!results) return 0;
			memcpy(results + size, path, pathSize);
			TFree(path);

			num++;
			size += pathSize;
		}
	}
	closedir(dir);
#endif

	return num;
}

size_t TFileSysListSubDirectoryNames(char **results, const char *_dir) {
	size_t idx = 0;

	/*#ifdef _WINDOWS
		struct _finddata_t thisfile;
		char *dir = TFileSysConcatPaths(_dir, "*", NULL);
		long fileindex = _findfirst(dir, &thisfile);

		if (fileindex != -1) {
		while (!_findnext(fileindex, &thisfile)) {
		if (thisfile.name[0] != '.' && thisfile.attrib & _A_SUBDIR) {
		char *newname = TStringCopyO(thisfile.name);

		output = TRAlloc(output, sizeof(char *) * (idx + 1));
		output[idx] = newname;
		idx++;
		}
		}
		}

		TFree(dir);
		#else
		DIR *dir = opendir(_dir);
		struct dirent *entry;

		if (!dir) return 0;
		for (; (entry = readdir(dir)) != NULL;) {
		if (entry->d_name[0] == '.')
		continue;

		char fullname[strlen(_dir) + strlen(entry->d_name) + 2];
		sprintf(fullname, "%s%s%s", _dir, _dir[0] ? "/" : "", entry->d_name);

		if (TFileSysIsDirectory(fullname)) {
		output = TRAlloc(output, sizeof(char *) * (idx + 1));
		output[idx] = strdup(fullname);
		idx++;
		}
		}
		closedir(dir);
		#endif*/
	return idx;
}

unsigned char TFileSysIsDirectory(const char *_fullPath) {
#ifdef _WINDOWS
	unsigned long dwAttrib = GetFileAttributes(_fullPath);
	return (dwAttrib != -1 && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
	struct stat s;
	int rc = stat(_fullPath, &s);
	if (rc != 0) return 0;
	return(s.st_mode & S_IFDIR);
#endif
}

unsigned char TFileSysFileExists(const char *_fullPath) {
	char *fixedPath = TFileSysFindCaseInsensitive(_fullPath);
	FILE *f = TFileSysOpen(fixedPath, "r");
	TFree(fixedPath);

	if (f) {
		fclose(f);
		return 1;
	}

	return 0;
}

unsigned char TFileSysFilesIdentical(const char *_name1, const char *_name2) {
	FILE *in1 = fopen(_name1, "r");
	FILE *in2 = fopen(_name2, "r");
	unsigned char rv = 1;
	unsigned char exitNow = 0;

	if (!in1 || !in2) {
		rv = 0;
		exitNow = 1;
	}

	while (!exitNow && !feof(in1) && !feof(in2)) {
		int a = fgetc(in1);
		int b = fgetc(in2);
		if (a != b) {
			rv = 0;
			exitNow = 1;
			break;
		}
	}

	if (!exitNow && feof(in1) != feof(in2))
		rv = 0;

	if (in1)fclose(in1);
	if (in2)fclose(in2);

	return rv;
}

unsigned char TFileSysIsWindowsPath(const char *path) {
	if (!path) return 0;

	return strchr(path, '\\') != 0;
}

unsigned char TFileSysIsFullPath(const char *path) {
	if (!path) return 0;

#ifdef _WINDOWS
	if(TFileSysIsWindowsPath(path)) return TStringIsAlphabetCharacter(*path) && *(path + 1) == ':' && (*(path + 2) == '/' || *(path + 2) == '\\');
#endif

	return *path == '/';
}

char *TFileSysGetWorkingDirectory(void) {
	char *path = TAllocNData(char, TBUFSIZE);
	if (!getcwd(path, FILENAME_MAX * sizeof(char))) {
		TFree(path);
		TErrorSet(T_ERROR_UNKNOWN);
		return 0;
	}
	return path;
}

char *TFileSysFixFolderPath(const char *path) {
	char *newPath, *ptr, *nPtr;
	const char *next;
	TSize size;
	char ending;

	if (!path) return 0;

	size = (strlen(path) + 1);
	ending = *(path + (size - 2));
	if (ending != '/' && ending != '\\')
		size++;

	newPath = TAllocNData(char, size);
	if (!newPath) return 0;
	nPtr = newPath + size - 1;
	*nPtr = 0;
	*(nPtr - 1) = '/';
	nPtr = newPath;

	next = path;
	while ((ptr = strchr(next, '\\'))) {
		*ptr = 0;
		size = strlen(next);
		*ptr = '\\';
		memcpy(nPtr, next, size * sizeof(char));
		nPtr += size;
		*(nPtr++) = '/';
		next = ptr + 1;
	}
	size = strlen(next);
	memcpy(nPtr, next, size * sizeof(char));

	return newPath;
}

char *TFileSysFixFilePath(const char *path) {
	char *newPath, *ptr, *nPtr;
	const char *next;
	TSize size;

	if (!path) return 0;

	size = (strlen(path) + 1);
	newPath = TAllocNData(char, size);
	if (!newPath) return 0;
	*(newPath + size - 1) = 0;
	nPtr = newPath;

	next = path;
	while ((ptr = strchr(next, '\\'))) {
		*ptr = 0;
		size = strlen(next);
		*ptr = '\\';
		memcpy(nPtr, next, size * sizeof(char));
		nPtr += size;
		*(nPtr++) = '/';
		next = ptr + 1;
	}
	size = strlen(next);
	memcpy(nPtr, next, size * sizeof(char));

	return newPath;
}

static inline void TFileSysGetParentInternal(const char *buf) {
	char *finalSlash = strrchr(buf, '/');
	if (finalSlash) {
		if (*(finalSlash + 1) == '\x0') {
			char *tmp = finalSlash;
			*finalSlash = '\x0';
			finalSlash = strrchr(buf, '/');
			if (!finalSlash) {
				*tmp = '/';
				return;
			}
		}
		*(finalSlash + 1) = '\x0';
	}
}

char *TFileSysConcat(const char *_firstComponent, ...) {
	char *result, *ptr;
	va_list components;
	const char *component;
	TSize size;

	if (!_firstComponent) return 0;

	result = TStringCopy(_firstComponent);
	size = TStringSize(result);

	va_start(components, _firstComponent);
	while ((component = va_arg(components, const char *))) {
		if (!strcmp(component, "..")) {
			TFileSysGetParentInternal(result);
			size = TStringSize(result);
			result = TRAlloc(result, size);
			ptr = result + size - 1;

		} else if (strcmp(component, ".")) {
			TSize llen = strlen(component) + 1;
			size += llen * sizeof(char);
			result = TRAlloc(result, size);
			ptr = result + size - llen - 1;
			*(ptr++) = '/';
			memcpy(ptr, component, llen * sizeof(char));
		}
	}
	va_end(components);

	return result;
}

char *TFileSysConcatExt(const char *_firstComponent, ...) {
	char *result, *ptr;
	va_list components;
	const char *component, *peek;
	TSize size;

	if (!_firstComponent) return 0;

	result = TStringCopy(_firstComponent);
	size = TStringSize(result);

	va_start(components, _firstComponent);
	component = va_arg(components, const char *);
	while (component) {
		peek = va_arg(components, const char *);

		if (!strcmp(component, "..")) {
			TFileSysGetParentInternal(result);
			size = TStringSize(result);
			result = TRAlloc(result, size);
			ptr = result + size - 1;

		} else if (strcmp(component, ".")) {
			TSize llen = strlen(component) + 1;
			size += llen * sizeof(char);
			result = TRAlloc(result, size);
			ptr = result + size - llen - 1;
			*(ptr++) = peek ? '/' : '.';
			memcpy(ptr, component, llen * sizeof(char));
		}
		component = peek;
	}
	va_end(components);

	return result;
}

char *TFileSysGetParent(char *_fullFilePath) {
	TFileSysGetParentInternal(_fullFilePath);
	return _fullFilePath;
}

char *TFileSysGetDirectory(const char *_fullFilePath) {
	char *finalSlash;

#ifdef _WINDOWS
	if ((finalSlash = strrchr(_fullFilePath, '\\'))) {
		return TStringNCopy(_fullFilePath, finalSlash - _fullFilePath);
	}
#endif

	if ((finalSlash = strrchr(_fullFilePath, '/'))) {
		return TStringNCopy(_fullFilePath, finalSlash - _fullFilePath);
	}

	return 0;
}

char *TFileSysGetDirname(const char *_fullFilePath) {
	char *cpy;
	const char *start, *end;

	if (!_fullFilePath) return 0;

	end = strrchr(_fullFilePath, '/');
	if (!end) return 0;

	if (end == _fullFilePath) {
		start = 0;
	} else {
		const char *cur = _fullFilePath;
		start = _fullFilePath;
		while ((cur = strchr(cur, '/')) != end) { cur += 1; start = cur; }
	}

	if (!(end - start)) return 0;
	cpy = (char *)TAlloc(sizeof(char) * (end - start + 1));
	if (cpy) memcpy(cpy, start, end - start);
	cpy[end - start] = 0;

	return cpy;
}

const char *TFileSysGetFilename(const char *_fullFilePath) {
	const char *result = strrchr(_fullFilePath, '/');
	if (result) result++;
	return result;
}

const char *TFileSysGetExtension(const char *_fullFilePath) {
	const char *result = strrchr(_fullFilePath, '.');
	return result ? result + 1 : result;
}

char *TFileSysRemoveExtension(const char *_fullFileName) {
	char *dot = strrchr(_fullFileName, '.');
	if (dot) return TStringSCopy(_fullFileName, dot - _fullFileName);
	return 0;
}

char **TFileSysSplitPath(const char *path) {
	size_t len = strlen(path), split = 1, size;
	if (path[len] == '/') split = 2;

	return TStringRSplit(path, "/", &size, split);
}

char **TFileSysSplitPathFull(const char *path, size_t *size) {
	return TStringSplit(path, "/", size, 0);
}

TSize TFileSysGetFileSize(const char *path) {
#ifdef _WINDOWS
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (GetFileAttributesEx(path, GetFileExInfoStandard, &data)) {
		return data.nFileSizeLow;
	}
#else
	struct stat s;
	int rc = stat(path, &s);
	if (!rc)
		return s.st_size;
#endif
	return 0;
}

int TFileSysIsReadOnly(FILE *f) {
#ifdef _WINDOWS
#	ifdef COMPILER_MICROSOFT
#		if _MSC_VER < 1900
	return f->_flag & FILE_ATTRIBUTE_READONLY;
#		else
	char buf = 0;
	
	fread(&buf, 1, 1, f);
	fseek(f, -1, SEEK_CUR);
	if (fwrite(&buf, 1, 1, f)) {
		fseek(f, -1, SEEK_CUR);
		return 0;
	}
#		endif
#	endif
#else
	int fd = fileno(f);
	return fcntl(fd, F_GETFL) & O_RDONLY;
#endif

	return 1;
}

FILE *TFileSysOpen(const char *path, const char *mode) {
	FILE *f;
#ifdef _WINDOWS
	wchar_t *wPath, *wMode;

	if (!path) return 0;
	if (!mode) mode = "r";

	wPath = TStringToWideChar(path);
	wMode = TStringToWideChar(mode);

	f = _wfopen(wPath, wMode);

	TFree(wMode);
	TFree(wPath);


#else
	if (!path) return 0;
	if (!mode) mode = "r";

	f = fopen(path->content, mode);
#endif

	return f;
}

unsigned char TFileSysCreateDirectory(const char *path) {
#ifdef _WINDOWS
	wchar_t *wPath = TStringToWideChar(path);
	int result = _wmkdir(wPath);
	TFree(wPath);
	if (result == 0)									return 0;	// Directory was created
	if (result == -1 && errno == 17 /* EEXIST */)	return 0;	// Directory already exists
	return 1;
#else
	if (access(path.content, W_OK | X_OK | R_OK) == 0)
		return 0;
	return mkdir(path.content, 0755);
#endif
}

unsigned char TFileSysCreateDirectoryRecursively(const char *path) {
	char *p;
	unsigned char error = 0;

	if (!path) return 0;

	p = (char *)path;
	if (*p == '/') p++;

	while ((p = strchr(p, '/')) && !error) {
		char c = *p;
		*p = 0;
		if (!TFileSysIsDirectory(path))
			error = TFileSysCreateDirectory(path);
		*(p++) = c;
	}

	return error;
}

void TFileSysDelete(const char *path) {
#ifdef _WINDOWS
	wchar_t *wPath = TStringToWideChar(path);
	DeleteFileW(wPath);
	TFree(wPath);
#else
	unlink(path.content);
#endif
}

char *TFileSysFindCaseInsensitive(const char *_fullPath) {
#ifndef _LINUX
	return TStringCopy(_fullPath);
#else
	char *result;
	char *dir, *file;
	char *files = 0;
	TSize numfiles = 0;

	if (!_fullPath) return result;

	// Make our own copy of the result, since GetDirectoryPart
	// and GetFilenamePart use the same variable for temp
	// storage.
	dir = TFileSysGetDirectory(_fullPath);

	// No directory provided. Assume working directory.
	if (dir) file = TStringCopy(_fullPath);
	else {
		file = TStringCopy(TFileSysGetFilename(_fullPath));
	}
	numfiles = TFileSysListDirectory(&files, dir, file, 0);

	TFree(dir); TFree(file);
	dir = file = 0;

	// We shouldn't have found more than one match.
	if (numfiles > 1) {
		TFree(files);
		return 0;
	}

	// No results, so maybe the file does not exist.
	if (numfiles == 0) return _fullPath;

	// Copy the corrected path back, and prepare to return it.
	res = files[0];
	TFree(files);

	return res;
#endif
}
