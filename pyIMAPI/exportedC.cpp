#include "stdafx.h"

extern "C" PYIMAPI_API char *fadd(void *cpp_obj_ptr, char *filename, int add_dir)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	//only return const strings or pass strings back to free with delete [] etc
	return fs_obj->add(filename, add_dir);

}

extern "C" PYIMAPI_API char *fgetcwd(void *cpp_obj_ptr)
{ 
	//printf("in wrapper");
	//printf("%p\n", obj);
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;
	//printf("%p\n", obj);

	return fs_obj->getCWD();
}

extern "C" PYIMAPI_API void ffreecwd(char *cwd)
{
	delete[] cwd;
}

//create the iso file from the image
extern "C" PYIMAPI_API  void fcreateImage(void *cpp_obj_ptr)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	fs_obj->createISO();
}

extern "C" PYIMAPI_API  void fcloseImage(void *cpp_obj_ptr)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	fs_obj->close();	
}
extern "C" PYIMAPI_API void fdeleteIMAPI2FS(void *cpp_obj_ptr)
{
	delete cpp_obj_ptr;
}

// This is the constructor of a class that has been exported.
// see pyIMAPI.h for the class definition

extern "C" PYIMAPI_API  char **flist(void *cpp_obj_ptr)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	return fs_obj->list();
}

extern "C" PYIMAPI_API  LONG fcount(void *cpp_obj_ptr)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	return fs_obj->count();
}

extern "C" PYIMAPI_API  void *fcreateIMAPI2FS(char *filename, char *mode, char *disk_type, int bootable)
{
	//printf("about to new\n");
	//tested if this new is creating the issue calling help(pyIMAPI2FS) after pyIMAPI2FS.open()
	//did not prevent the crash when calling help(pyIMAPI2FS)
	void *cpp_obj_ptr = (void *)new CpyIMAPIObject(filename, mode, disk_type, bootable);
	//printf("after new\n");
	return cpp_obj_ptr;
}

extern "C" PYIMAPI_API int fexists(void *cpp_obj_ptr, char *filename)
{
		//reinterpret_cast
		CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

		//printf("in exists wrapper");

		return fs_obj->exists(filename);
}

/*
extern "C" PYIMAPI_API void fextract(void *cpp_obj_ptr, char *filename, char *opt_destpath) 
{
		CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

		//printf("in exists wrapper");

		fs_obj->extract(filename);

		return;
}
*/

extern "C" PYIMAPI_API char *wrapper_setvolumename(void *cpp_obj_ptr, char *volname)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	return fs_obj->set_volume_name(volname);
}

extern "C" PYIMAPI_API char *wrapper_getvolumename(void *cpp_obj_ptr)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	return fs_obj->get_volume_name();
}

extern "C" PYIMAPI_API void wrapper_delete_array(char *mem_to_free) 
{
	delete[] mem_to_free;
}

extern "C" PYIMAPI_API char *wrapper_setbootsector(void *cpp_obj_ptr, char *filename)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;
		
	return fs_obj->set_boot_sector(filename);
}

extern "C" PYIMAPI_API void ffreelist(void *cpp_obj_ptr, char **paths)
{
	//reinterpret_cast
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	fs_obj->freelist(paths);
}
extern "C" PYIMAPI_API int fremove(void *cpp_obj_ptr, char *filename) 
{
	//reinterpret_cast
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	return fs_obj->remove(filename);

}


extern "C" PYIMAPI_API char *fmkdir(void *cpp_obj_ptr, char *filename)
{
	//printf("in fmkdir\n");
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;
	//printf("%p\n", obj);
	return fs_obj->mkdir(filename);
}

extern "C" PYIMAPI_API char *fchdir(void *cpp_obj_ptr, char *filename)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)cpp_obj_ptr;

	return fs_obj->setCWD(filename);
}