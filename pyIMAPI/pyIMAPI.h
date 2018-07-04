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
	PYIMAPI_API void *fcreateIMAPI2FS(char *filename, char *mode, char *disk_type, int bootable);
	PYIMAPI_API void fdeleteIMAPI2FS(void *);
	PYIMAPI_API char *fgetcwd(void *cpp_obj_ptr);
	PYIMAPI_API void ffreecwd(char *cwd);
	PYIMAPI_API char *fmkdir(void *cpp_obj_ptr, char *path);
	PYIMAPI_API char *fchdir(void *cpp_obj_ptr, char *path);
	PYIMAPI_API void fcloseImage(void *cpp_obj_ptr);
	PYIMAPI_API int fexists(void *cpp_obj_ptr, char *filename);
	PYIMAPI_API char **flist(void *cpp_obj_ptr);
	PYIMAPI_API void ffreelist(void *cpp_obj_ptr, char **paths);
	PYIMAPI_API char *fadd(void *cpp_obj_ptr, char *filename, int add_dir);
	PYIMAPI_API int fremove(void *cpp_obj_ptr, char *filename);

	PYIMAPI_API char *wrapper_setvolumename(void *cpp_obj_ptr, char *vol_name);
	PYIMAPI_API char *wrapper_getvolumename(void *cpp_obj_ptr);
	PYIMAPI_API void wrapper_delete_array(char *mem_to_delete);

	PYIMAPI_API char* wrapper_setbootsector(void *cpp_obj_ptr, char *filename);

	//extract is handled by reading layer (powershell or 7zip, iso isnt written till file close)
	//PYIMAPI_API int fextract(void *cpp_obj_ptr, char *isofile_member, char *dest_system_path);
#ifndef LONG
//try to match VS LONG #define to the likely type
	PYIMAPI_API long fcount(void *cpp_obj_ptr);
#else
	PYIMAPI_API LONG fcount(void *cpp_obj_ptr);
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
	IBootOptions*		BootOptionsInstance;
	FsiFileSystems		fs_type;
	IFsiItem*			item_iter;

	IStream*	output_file;

	IMAPI_MEDIA_PHYSICAL_TYPE imapi_disk_type;

public:
	CpyIMAPIObject(char *filename, char *mode, char *disk_type, int bootable);
	~CpyIMAPIObject(void);
	
	char*		set_volume_name(char* volumename);
	char*		get_volume_name();
	char*		add(char* filename, int add_dir);//, char *dest_filename);
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

	char*		set_boot_sector(char *filename);

	//return IsoInfo object
	void*		next();

};

/*
extern PYIMAPI_API int npyIMAPI;
*/
#endif