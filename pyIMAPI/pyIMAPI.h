// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PYIMAPI_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PYIMAPI_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#ifdef PYIMAPI_EXPORTS
#define PYIMAPI_API __declspec(dllexport)
#else
#define PYIMAPI_API __declspec(dllimport)
#endif
#ifdef __cplusplus
extern "C"
{
#endif
	PYIMAPI_API void *fcreateIMAPI2FS(char *filename, char *mode, char *disk_type);
	PYIMAPI_API void fdeleteIMAPI2FS(void *);
	PYIMAPI_API char *fgetcwd(void *obj);
	PYIMAPI_API void ffreecwd(char *cwd);
	PYIMAPI_API char *fmkdir(void *obj, char *path);
	PYIMAPI_API char *fchdir(void *obj, char *path);
	PYIMAPI_API void fcloseImage(void *obj);
	PYIMAPI_API int fexists(void *obj, char *filename);
	PYIMAPI_API char **flist(void *obj);
	PYIMAPI_API void ffreelist(void *obj, char **paths);
	PYIMAPI_API char *fadd(void *obj, char *filename);
	PYIMAPI_API int fremove(void *obj, char *filename);
	//extract is handled by reading layer (powershell or 7zip, iso isnt written till file close)
	//PYIMAPI_API int fextract(void *obj, char *isofile_member, char *dest_system_path);
#ifndef LONG
//try to match VS LONG #define to the likely type
	PYIMAPI_API long fcount(void *);
#else
	PYIMAPI_API LONG fcount(void *);
#endif
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus

#include <imapi2fs.h>
#include <imapi2fserror.h>

// This class is exported from the pyIMAPI.dll
class  CpyIMAPIObject 
{
	IFsiDirectoryItem*	root;
	IFsiDirectoryItem*	current_directory;
	IFileSystemImage*	FileSystem;
	FsiFileSystems		fs_type;
	IFsiItem*			item_iter;

	IStream*	output_file;

	IMAPI_MEDIA_PHYSICAL_TYPE imapi_disk_type;

public:
	CpyIMAPIObject(char *filename, char *mode, char *disk_type);
	~CpyIMAPIObject(void);
	// TODO: add your methods here.
	void		set_volume_name(char* volumename);
	char*		get_volume_name();
	char*		add(char* filename);//, char *dest_filename);
	char*		getCWD();
	wchar_t*	getwCWD();
	char*		setCWD(char *path);
	char*		mkdir(char *path);
	bool		exists(char* filename);
	char**		list();
	void		freelist(char **list);
	void		createISO();
	LONG		count();
	void		close();
	int			remove(char *filename);

	int			extract(char *filename, char *dest_system_path);

	//return IsoInfo object
	void*		next();
};

/*
extern PYIMAPI_API int npyIMAPI;
*/
#endif