#include "stdafx.h"

extern "C" PYIMAPI_API char *fadd(void *obj, char *filename)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	//only return const strings or pass strings back to free with delete [] etc
	return fs_obj->add(filename);

}

extern "C" PYIMAPI_API char *fgetcwd(void *obj)
{ 
	//printf("in wrapper");
	//printf("%p\n", obj);
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;
	//printf("%p\n", obj);

	return fs_obj->getCWD();
}

extern "C" PYIMAPI_API void ffreecwd(char *cwd)
{
	delete[] cwd;
}

//create the iso file from the image
extern "C" PYIMAPI_API  void fcreateImage(void *obj)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	fs_obj->createISO();
}

extern "C" PYIMAPI_API  void fcloseImage(void *obj)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	fs_obj->close();	
}
extern "C" PYIMAPI_API void fdeleteIMAPI2FS(void *obj)
{
	delete obj;
}

// This is the constructor of a class that has been exported.
// see pyIMAPI.h for the class definition

extern "C" PYIMAPI_API  char **flist(void *obj)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	return fs_obj->list();
}

extern "C" PYIMAPI_API  LONG fcount(void *obj)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	return fs_obj->count();
}

extern "C" PYIMAPI_API  void *fcreateIMAPI2FS(char *filename, char *mode, char *disk_type)
{
	//printf("about to new\n");
	//tested if this new is creating the issue calling help(pyIMAPI2FS) after pyIMAPI2FS.open()
	//did not prevent the crash when calling help(pyIMAPI2FS)
	void *object = (void *)new CpyIMAPIObject(filename, mode, disk_type);
	//printf("after new\n");
	return object;
}

extern "C" PYIMAPI_API int fexists(void *obj, char *filename)
{
		//reinterpret_cast
		CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

		//printf("in exists wrapper");

		return fs_obj->exists(filename);
}

/*
extern "C" PYIMAPI_API void fextract(void *obj, char *filename, char *opt_destpath) 
{
		CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

		//printf("in exists wrapper");

		fs_obj->extract(filename);

		return;
}
*/

extern "C" PYIMAPI_API void ffreelist(void *obj, char **paths)
{
	//reinterpret_cast
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	fs_obj->freelist(paths);
}
extern "C" PYIMAPI_API int fremove(void *obj, char *filename) 
{
	//reinterpret_cast
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	return fs_obj->remove(filename);

}


extern "C" PYIMAPI_API char *fmkdir(void *obj, char *filename)
{
	//printf("in fmkdir\n");
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;
	//printf("%p\n", obj);
	return fs_obj->mkdir(filename);
}

extern "C" PYIMAPI_API char *fchdir(void *obj, char *filename)
{
	CpyIMAPIObject *fs_obj = (CpyIMAPIObject *)obj;

	return fs_obj->setCWD(filename);
}