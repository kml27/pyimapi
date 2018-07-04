//Kenneth Long
//9.17.2016
//IMAPI2 File System Python Module

#include <windows.h>
#include <Shlwapi.h>
#include <Python.h>
#include "pyIMAPI.h"
#include <time.h>

#pragma comment(lib, "shlwapi.lib")

#define error_name						"IMAPI2Error"
#define module_name						"pyIMAPI"
#define error_type_name					module_name"."error_name

#define filesystem_type_name			"FileSystem"

#if PY_MAJOR_VERSION == 2
staticforward PyTypeObject pyIMAPI2FS_type;
#endif
#if PY_MAJOR_VERSION >= 3
//https://docs.python.org/3/howto/cporting.html
static PyTypeObject pyIMAPI2FS_type;

#define PyString_FromString(x)		PyUnicode_FromString(x)
#define PyString_FromFormat(x, ...) PyUnicode_FromFormat(x, __VA_ARGS__)

#endif
PyDoc_STRVAR(module_doc, "Windows IMAPI2 File System Python Module\n\
				"filesystem_type_name":\t\tan IMAPI2 FileSystem, do not use this type directly, use "module_name".open() instead\n\
				"error_name":\t\ta catch all exception for any errors\n");

/*
maybe
FileSystem:	`` with control over underlying structure
DataCD:		an IMAPI2 FileSystem based object\n\
AudioCD:	an IMAPI2 Raw Format DiscAtOnce compatible object\n\
*/

/*
#define datacd_filesystem_type_name		"DataCD"
#define audiocd_filesystem_type_name	"AudioCD"
*/

/*#if PY_MAJOR_VER == 2
static PyObject *imapi2fs;
static PyObject *imapi2fs_error;
#endif
*/

struct module_state {
	PyObject *imapi2fs;
	PyObject *imapi2fs_error;
	//???
	//hows it handled in py3
	//imapi2fs_type?
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
static struct module_state _state;

#define GETSTATE(m) (&_state)
#endif


#define object_check(t) (Py_TYPE(t) == &pyIMAPI2FS_type)

//arguably global statics are a bad practice
//less than a reg (slower than wasting a register... flags if more needed?)
static char no_mount = 0;

static HMODULE external_dll;

//considered bad practice by some... const static int... 
#define MAX_CLL 8191

//const static int MAX_CLL = 8191;

enum _objType
{
	//imapi,		//creating an iso image in xp sp2+ (or "appending" using intermediate tmp file)
	//mount,		//using powershell in win8+ to read the file ("ro" mode)
	
	internal,	//all internal IMAPI and PowerShell
	external,	//using ISO 7z for read
};

typedef enum _objType objType;

typedef struct {
	PyObject_HEAD
		PyObject		*filename_str;
		char			*filename;
		char			tmp_filename[MAX_PATH];
		char			mode[3];
		PyObject		*mode_str;
		void			*IMAPI2FS_Object;
		objType			type;
		char			drive[3];
		//use unicode and prepend "\\?\" to exceed max_path on newer windows before 10, 10 doesnt require it...
		char			path[MAX_PATH];
		int				open;
		PyObject		*disk_type;

} imapi2fs_object;


//primarily intended for use with powershell ISO mounting, copying and unmounting
int flush_and_call_system(char *command)
{

	//printf("%s\n", command);

	//You must explicitly flush�by using fflush or _flushall�or close any stream before you call system.//
	//https://msdn.microsoft.com/en-us/library/277bwbdz.aspx

	fflush(stdin);
	fflush(stdout);
	fflush(stderr);

	//ShellExecute(NULL, NULL, "powershell", mount_ps, NULL, )
	return system(command);

}

static imapi2fs_object *get_imapi2fs(PyObject *self)
{
	if (!object_check(self))
		return NULL;

	return (imapi2fs_object *)self;
}

//memberdef
//method name, type, offset, 0, description
static PyObject *imapi2fs_add(PyObject *self, PyObject *args, PyObject *keywds)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	static char *kwlist[] = { "path", "only_add_dir_contents", NULL };

	char *result = "";

	char *filename = NULL;

	int add_dir = 0;

	//if (obj->type == mount)
	if(!_strcmpi(obj->mode, "r"))
	{
		PySys_WriteStdout("Mounted filesystem is read-only, cannot modiy image\n");
		Py_RETURN_FALSE;
	}
	//printf("add");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|b:keywords", kwlist, &filename, &add_dir))
		return NULL;


	if (result = fadd(obj->IMAPI2FS_Object, filename, add_dir?-1:0))
	{
		PySys_WriteStdout(result);
		Py_RETURN_FALSE;
	}

	Py_RETURN_TRUE;
}

static PyObject *imapi2fs_mkdir(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *filename = NULL;

	//printf("mkdir");

	if (obj == NULL)
		Py_RETURN_NONE;
	
	//if (obj->type == mount)
	if(!_strcmpi("r", obj->mode))
	{
		PySys_WriteStdout("Mounted filesystem is read-only, cannot modiy image\n");
		Py_RETURN_FALSE;
	}

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	fmkdir(obj->IMAPI2FS_Object, filename);

	Py_RETURN_NONE;
}
static PyObject *imapi2fs_chdir(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *filename = NULL;

	//printf("chdir\n");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	switch (obj->type)
	{
		//case imapi:
		case internal:
		
		//if mode not read
		if (_strcmpi(obj->mode, "r"))
		{
			if (fchdir(obj->IMAPI2FS_Object, filename) == (void*)NULL)
				Py_RETURN_TRUE;

			//break;
		}
		else //(mode is read only)
		//case mount:
		{
			//exists would use mount_exists(obj, filename, ANY, concat)
			
			//chdir would use if(mount_exists(obj, filename, DIR, concat))
			struct stat info;
			char fullpath[MAX_PATH];
			sprintf_s(fullpath, MAX_PATH, "%s:\\%s\\%s", obj->drive, obj->path, filename);
			if (stat(fullpath, &info) == S_OK)
			{
				if (info.st_mode & S_IFDIR)
				{
					sprintf_s(obj->path, MAX_PATH, "%s\\%s", obj->path, filename);
					Py_RETURN_TRUE;
				}
			}
			
		}

		break;
		case external:
			break;
	}

	Py_RETURN_FALSE;
}

static PyObject *imapi2fs_getcwd(PyObject *self, PyObject *noarg)
{
	imapi2fs_object *obj = get_imapi2fs(self);
	PyObject *ret = NULL;
	char *cwd = NULL;

	//printf("obj %p", obj);
	//printf("getcwd");

	if (obj == NULL)
		Py_RETURN_NONE;

	switch (obj->type)
	{
		//case imapi:
		case internal:
			//not read
		if(_strcmpi(obj->mode, "r"))
		{
			cwd = fgetcwd(obj->IMAPI2FS_Object);
			

			ret = PyString_FromString(cwd);

			ffreecwd(cwd);

			if (ret == NULL)
				Py_RETURN_NONE;
		}
		else //if read only
		{
			if (strlen(obj->path))
				ret = PyString_FromString(obj->path);
			else
				ret = PyString_FromString("\\");
		}

		break;
		case external:
		{
			break;
		}
	}
	//printf("building pyvalue");
	return ret;
}

//char **dir_list(imapi2fs_object *obj) 
void add_existing_files(imapi2fs_object *obj)
{
	int count=0, i=0;
	char temp[MAX_PATH + 4];
	WIN32_FIND_DATAA find;
	HANDLE hFind;
	BOOL avail = FALSE;
	//char **list;

	/*
	When windows version checking is implemented, new versions of windows allow longer filenames
	https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
	*/

	sprintf_s(temp, MAX_PATH + 4, "%s:\\%s\\*", obj->drive, obj->path);
	
	//printf("checking %s for existing files\n", temp);
	
/*	hFind = FindFirstFileA(temp, &find);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			count++;
		} while (FindNextFileA(hFind, &find));

		FindClose(hFind);
	}
*/
	hFind = FindFirstFileA(temp, &find);

	//list = malloc(sizeof(char *) * count);
	//memset(list, 0, sizeof(char *) * count);

	//printf("%d\n", count);
	//list = PyList_New(count);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			//int len = strlen(find.cFileName) + 1;
			
			//printf("%s\n", find.cFileName);
			
			sprintf_s(temp, MAX_PATH, "%s:\\%s\\%s", obj->drive, obj->path, find.cFileName);
			
			//printf("%s\n", temp);

			//list[i] = malloc(len);
			//strcpy_s(list[i++], len, find.cFileName);
			
			//add dir contents has no effect for add file
			fadd(obj->IMAPI2FS_Object, temp, FALSE);
			
		} while ((avail = FindNextFileA(hFind, &find)));// && i < count);

		if (avail /*&& i == count*/)
		{
			PySys_WriteStderr("Internal error buidling list, more items in image than found when enumerating\n");
		}
/**/
		FindClose(hFind);
	}
	else
	{
		//printf("no files\n");
	}

	//printf("finished iterating existing files\n");
}

static PyObject *imapi2fs_list(PyObject *self, PyObject *args, PyObject *keywds)
{
	
	imapi2fs_object *obj = get_imapi2fs(self);

	static char *kwlist[] = { "verbose", "recursive", NULL };

	int number_files=0;
	char **paths;
	int count = 0;
	int i=0;
	//get number of files
	
	char verbose = 0;
	char recursive = 0;

	PyObject *list = NULL, *item = NULL;

	//printf("entering list()");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|bb:keywords", kwlist, &verbose, &recursive))
		return NULL;

	//verbose... list only ordinary or dir? is rock ridge supported? El Torito is... 
	//https://docs.microsoft.com/en-us/windows/desktop/api/imapi2fs/nn-imapi2fs-ibootoptions



	switch (obj->type)
	{
		//case imapi:
		case internal:
		//not read only mode
		if(_strcmpi(obj->mode, "r"))
		{
			//printf("calling count");
			count = fcount(obj->IMAPI2FS_Object);

			//printf("calling list\n");
			paths = flist(obj->IMAPI2FS_Object);

			//get each full file path
			//printf("buidling python list\n");

			list = PyList_New(count);

			if (count > 0 && list != NULL)
			{
				//printf("%d", count);
				for (i = 0; i < count; i++)
				{
					//printf("%s", paths[i]);
					item = PyString_FromString(paths[i]);
					PyList_SET_ITEM(list, i, item);
				}
			}

			ffreelist(obj->IMAPI2FS_Object, paths);

			//break;
		}
		
		//case mount:
		else
		{
			char temp[MAX_PATH+4];
			WIN32_FIND_DATAA find;
			HANDLE hFind;
			BOOL avail = FALSE;

			/*
			When windows version checking is implemented, new versions of windows allow longer filenames
			https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
			*/

			sprintf_s(temp, MAX_PATH+4, "%s:\\%s\\*", obj->drive, obj->path);
			//printf("%s\n", temp);
			hFind = FindFirstFileA(temp, &find);
			
			if (hFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					count++;
				} while (FindNextFileA(hFind, &find));

				FindClose(hFind);
			}

			hFind = FindFirstFileA(temp, &find);
			
			//printf("%d\n", count);
			list = PyList_New(count);

			if (hFind != INVALID_HANDLE_VALUE)
			{				
				do
				{		
					//printf("%s\n", find.cFileName);
					item = PyString_FromString(find.cFileName);
					PyList_SET_ITEM(list, i++, item);

				} while ((avail = FindNextFileA(hFind, &find)) && i < count);

				if (avail && i == count)
				{
					PySys_WriteStderr("Internal error buidling list, more items in image than found when enumerating\n");
				}

				FindClose(hFind);
			}
			else
			{
				//printf("no files\n");
			}
		}
		break;

		case external:
		{
			break;
		}
	}

	return list;
}

static PyObject *imapi2fs_extract(PyObject *self, PyObject *args, PyObject *keywds)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	static char *kwlist[] = {"member","path",NULL};

	char *isofile_member = "";
	char *system_path = "";

	char copy_file[MAX_CLL];

	if (obj == NULL)
	{
		Py_RETURN_NONE;
	}

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|s:keywords", kwlist, &isofile_member, &system_path))
	{
		Py_RETURN_NONE;
	}

	//printf("%s %s\n", isofile_member, system_path);

	//if a system path was supplied, but does not exist

	/*if (system_path && !PathFileExistsA(system_path))
	{
		CreateDirectoryA(system_path, NULL);
	}*/

	switch (obj->type)
	{
		case internal:
		{
			if (!_strcmpi(obj->mode, "w"))
			{
				PySys_WriteStdout("Mounted filesystem is write-only, cannot read image\n");
				Py_RETURN_FALSE;
			}

			if (!strcmp(system_path, ""))
			{
				system_path = ".";
			}

			//if (!fextract(obj->IMAPI2FS_Object, isofile_member, system_path))
			//	Py_RETURN_FALSE;

			sprintf_s(copy_file, MAX_CLL, "copy \"%s:\\%s\" \"%s\" > nul", obj->drive, isofile_member, system_path);

			//printf("%s\n", copy_file);

			flush_and_call_system(copy_file);

			break;
		}

		case external:
		{
			break;
		}
	}

	Py_RETURN_TRUE;
}

static PyObject *imapi2fs_exists(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *filename = NULL;
	//printf("exists");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	switch (obj->type)
	{
		//case imapi:
		case internal:
			//not read only
		if(_strcmpi("r", obj->mode))
		{
			if (fexists(obj->IMAPI2FS_Object, filename))
			{
				Py_RETURN_TRUE;
			}
			break;
		}
		else //read only
		//case mount:
		{
			struct stat info;
			char fullpath[MAX_PATH];


			sprintf_s(fullpath, MAX_PATH, "%s:\\%s\\%s", obj->drive, obj->path, filename);
			//printf("%s\n", fullpath);

			printf("using stat (ro mode) with path %s\n", fullpath);


			if (stat(fullpath, &info) == S_OK)
			{
				//printf("stat returned 0, file info found\n");
				Py_RETURN_TRUE;
			}
			break;
		}

		case external:
		{
			break;
		}
	}

	Py_RETURN_FALSE;
}

static PyObject *imapi2fs_close(PyObject *self, PyObject *noarg)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	//https://msdn.microsoft.com/en-us/library/277bwbdz.aspx
	//C / C++ Language and Standard Libraries > C Run-Time Library Reference > Alphabetical Function Reference
	//system, _wsystem

	//If command is NULL and the command interpreter is found, returns a nonzero value.
	char *command = NULL;

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!obj->open)
	{
		//should this throw an exception?
		Py_RETURN_FALSE;
	}

	obj->open = FALSE;

	switch (obj->type)
	{
		case internal://imapi:
		
			if (obj != NULL)
			{

				//if a or w (not "r")
				if (_strcmpi(obj->mode, "r"))
				{
					//printf("calling dll to close image");
					fcloseImage(obj->IMAPI2FS_Object);

				}
				//break;

				//case mount:
				//if a or r (not "w")
				if (_strcmpi(obj->mode, "w"))
				{
					char dismount_ps[MAX_CLL];

					//printf("%s\n", obj->filename);
					sprintf_s(dismount_ps, MAX_CLL, "powershell -Command \"Dismount-DiskImage -Image \"%s\"\"", obj->filename);

					flush_and_call_system(dismount_ps);
				}

				//if append mode
				if (!_strcmpi("a", obj->mode))
				{
					char command[MAX_PATH];

					sprintf_s(command, MAX_PATH, "copy /y %s %s > nul", obj->tmp_filename, obj->filename);

					flush_and_call_system(command);

					sprintf_s(command, MAX_PATH, "del /q %s > nul", obj->tmp_filename);

					flush_and_call_system(command);
				}

			}
		break;

		case external:
			break;
	}

	//printf("returning");

	//obj->open = 0;

	Py_RETURN_TRUE;
}

static PyObject *imapi2fs_remove(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *filename = NULL;
	int result=0;
	//printf("remove");

	if (obj == NULL)
		Py_RETURN_NONE;
	
	//if (obj->type == mount)
	if(!_strcmpi(obj->mode, "r"))
	{
		PySys_WriteStdout("Mounted filesystem is read-only, cannot modiy image\n");
		Py_RETURN_FALSE;
	}

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	//nonzero success, zero fail (TRUE, FALSE)
	result = fremove(obj->IMAPI2FS_Object, filename);

	if(result)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;

}


static PyObject *imapi2fs_getdisktype(PyObject *self, PyObject *noarg)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	return obj->disk_type;

}

static PyObject *imapi2fs_setvolumename(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *vol_name = NULL;

	if (!PyArg_ParseTuple(args, "s", &vol_name))
		return NULL;

	wrapper_setvolumename(obj->IMAPI2FS_Object, vol_name);

	Py_RETURN_FALSE;

	Py_RETURN_TRUE;

}

static PyObject *imapi2fs_getvolumename(PyObject *self, PyObject *noargs)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *vol_name = wrapper_getvolumename(obj->IMAPI2FS_Object);
	
	PyObject *PyVolName = PyString_FromString(vol_name);
	
	wrapper_delete_array(vol_name);

	return PyVolName;
}

static PyObject *imapi2fs_getaccessmode(PyObject *self, PyObject *noargs)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	return obj->mode_str;
}

static PyObject *imapi2fs_setbootsector(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *image_filename = NULL;
	char *result = NULL;

	if (!PyArg_ParseTuple(args, "s", &image_filename))
		return NULL;

	result = wrapper_setbootsector(obj->IMAPI2FS_Object, image_filename);

	if (result) {
		return PyString_FromString(result);
	}

	Py_RETURN_TRUE;
}

//methods array with which to set tp_methods struct member
static PyMethodDef FileSystem_methods[] =	{	//name, function, args (METH_NOARGS, METH_VARARGS, METH_VARARGS|METH_KEYWORDS), description
												{"add",			(PyCFunction)imapi2fs_add,	METH_VARARGS | METH_KEYWORDS, PyDoc_STR("add a file to the ISO filesystem") },
												{"mkdir",		(PyCFunction)imapi2fs_mkdir, METH_VARARGS, PyDoc_STR("make a directory in the ISO filesystem") },
												{"chdir",		(PyCFunction)imapi2fs_chdir, METH_VARARGS, PyDoc_STR("change current working directory in ISO filesystem")},
												{"getcwd",		(PyCFunction)imapi2fs_getcwd, METH_NOARGS, PyDoc_STR("return current wording directory in ISO filesystem")},
												{"list",		(PyCFunction)imapi2fs_list, METH_VARARGS | METH_KEYWORDS, PyDoc_STR("return list [] of files in ISO filesystem")},
												{"extract",		(PyCFunction)imapi2fs_extract, METH_VARARGS | METH_KEYWORDS, PyDoc_STR("extract a file from the ISO filesystem, optional 2nd arg destination path, e.g. e.extract(\"test\", path=\"c:\\temp\\test.out\")") },
												{"exists",		(PyCFunction)imapi2fs_exists, METH_VARARGS, PyDoc_STR("check if the specified file exists in the ISO filesystem")},
												{"close",		(PyCFunction)imapi2fs_close, METH_NOARGS, PyDoc_STR("close this ISO filesystem")},
												{"remove",		(PyCFunction)imapi2fs_remove, METH_VARARGS, PyDoc_STR("remove the specified file from the ISO filesystem")},
												{"getdisktype", (PyCFunction)imapi2fs_getdisktype, METH_NOARGS, PyDoc_STR("Return the string representation of specified physical media type")},
												{"setvolumename", (PyCFunction)imapi2fs_setvolumename, METH_VARARGS, PyDoc_STR("Set the name of the ISO or disk (normally only displayed by the OS when the disk is mounted)") },
												{"getvolumename", (PyCFunction)imapi2fs_getvolumename, METH_NOARGS, PyDoc_STR("Get the name of the ISO or disk (normally only displayed by the OS when the disk is mounted)") },
												{"getaccessmode", (PyCFunction)imapi2fs_getaccessmode, METH_NOARGS, PyDoc_STR("Get the file access mode i.e. \"r\" (read), \"w\" (write) or \"a\" (append)") },
												{"setbootsector", (PyCFunction)imapi2fs_setbootsector, METH_VARARGS, PyDoc_STR("Provide a bootable image for the boot sectors of the disk, image must be opened with open(bootable=True)") },
												{NULL}//, NULL }//SENTINEL *ml_name must be NULL 
												//{ NULL, NULL, 0, NULL }//alternate sentinel
											};
									

//static PySequenceMethods FileSystem_sequence_methods[]={}
static imapi2fs_object *imapi2fs_explain(PyTypeObject *type, PyObject *arg)
{
	PySys_WriteStdout("FileSystem objects cannot be instantiated directly, use "module_name".open()");
	return NULL;
}

//replacing with only use open
static imapi2fs_object *imapi2fs_object_new(PyTypeObject *type, PyObject *arg)
{	//ptr to modules object type
	imapi2fs_object *self;

	//printf("about to use tp_alloc\nptr address is %p", type->tp_alloc);

	//allocate a new object, referencing static object instance for typechecking
	self = (imapi2fs_object *) type->tp_alloc(type, 0);

	//printf("tp_alloc'd");

	// NO INCREF DECREF 

	//did the allocation succeed
	if (self == NULL)
	{
		Py_DECREF(self);
		return NULL;
	}
	
	self->IMAPI2FS_Object = NULL;


	//Generic is supposed to memset...
#if PY_MAJOR_VERSION == 2
	//printf("about to memset");
	memset(self->drive, '\0', 2);
#endif

	//printf("initializing member vars");

	self->filename = NULL;
	self->open = 0;
	self->type = internal;//imapi;

	return self;
}

void store_filename(imapi2fs_object *obj, char *filename)
{
	PyObject *str = NULL;
	
	obj->filename = malloc(strlen(filename) + 1);
	memcpy(obj->filename, filename, strlen(filename) + 1);

	str = PyString_FromString(filename);

	obj->filename_str = str;

}

#define TIME_STR_LEN 128

static PyObject *imapi2fs_open(PyObject *self, PyObject *args, PyObject *keywds)
{
	//https://docs.python.org/2.7/extending/extending.html#keyword-parameters-for-extension-functions
	
	time_t ctime;
	struct tm tm_localtime;
	char sz_time[TIME_STR_LEN];

	char write_filename[MAX_PATH];
	char *read_filename = "default.iso";

	char *filename = NULL;

	char *mode = "a";
	FILE *handle = NULL;
	void *fsi = NULL;

	imapi2fs_object *obj = NULL;
	PyObject *str = NULL;

	static char *kwlist[] = {"filename", "mode", "disk_type", "bootable", NULL};

	objType type = internal;//imapi;

	char *disk_type = "CD";

	char *mount_drive = "z:\\";

	char path[MAX_PATH];
	char command[MAX_CLL];
	char fullpath[MAX_CLL];
	char drive[2];
	FILE *newdrive;
	int bootable = 0;

	char **existing_paths = NULL;
	unsigned int path_index = 0;

	int res = 0;

	char *c = NULL;

	time(&ctime);
	
	localtime_s(&tm_localtime, &ctime);
	
	//param 2, size in words... since wchar_t should be word sized, these should match
	asctime_s(sz_time, TIME_STR_LEN, &tm_localtime);

	//replace ' ' with '_'
	for (c = sz_time; c = strchr(sz_time, ' '); c++) 
	{
		*c = '_';
	}
	
	//replace ':' with '.'
	for (c = sz_time; c = strchr(sz_time, ':'); c++)
	{
		*c = '.';
	}

	sz_time[strlen(sz_time)-1] = '\0';

	sprintf_s(write_filename, MAX_PATH, "%s.iso", sz_time);

	//printf("parsing keywords\n");
	//:open
	//:new_noddy
	/*args should be filename and file open mode*/
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|sssb:keywords", kwlist, &read_filename, &mode, &disk_type, &bootable))
	{
		return NULL;
	}
	
	//printf("finished parsing keywords\n");

	//if append and no file, just use write (more efficient)
	if (!_strcmpi(mode, "a") && !PathFileExistsA(read_filename)) {
		mode = "w";

		//printf("changing mode from a to %s\n", mode);
	}

	//"w" => overwrite existing file
	if (!strcmp(mode, "w"))
	{
		//test read_filename=""
		memcpy_s(write_filename, MAX_PATH, read_filename, strlen(read_filename)+1);

		//printf("setting write_filename to %s\n", write_filename);
	}

	//"a" we need to create a temporary file to write to until "everything if finalized" so we dont loose the previous file
	//"w" will just overwrite the existing file (using the above code to copy the provided read_filename, to the write filename)
	//not read only
	if (_strcmpi(mode, "r"))
	{
		//printf("about to create internal object w/ filename %s\n", write_filename);
		
		fsi = fcreateIMAPI2FS(write_filename, mode, disk_type, bootable);
		
		//the object representation will present this file as the provided write_filename (in append mode this will be updated in the next branch)
		filename = write_filename;

		//printf("self is %p\n", fsi);
	}

	//not write only
	if ( _strcmpi(mode, "w") )
	{
		//type = internal;//mount;
		//if using windows 8 or later (or powershell is installed)
		//disks may be read by using powershell to mount the disk image

		GetCurrentDirectoryA((DWORD)MAX_PATH, path);

		sprintf_s(fullpath, MAX_PATH, "%s\\%s", path, read_filename);
	
		read_filename = fullpath;
		//printf("%s", filename);

//CHECK FOR fileexists here for (better?) robustness
		if (PathFileExistsA(read_filename)) {

			//in read mode, this is the filename provided by the user if any
			//in append mode, the previously set temporary filename (based on the localtime)
			filename = read_filename;

			sprintf_s(command, MAX_CLL, "powershell -Command \"Mount-DiskImage -Image \"%s\"\"", filename);
			//printf("%s : %s",command);

			res = flush_and_call_system(command);

			//0 == success
			if (!res)
			{

				sprintf_s(command, MAX_CLL, "powershell -Command \"Get-DiskImage -ImagePath \"%s\" | Get-Volume | Select -ExpandProperty DriveLetter\" > newdrive", filename);

				//could also use DevicePath for more robust handling when implementing wcs/unicode
				//powershell -Command "Get-DiskImage -ImagePath "c:\development\pyimapi2\pyimapi\default.iso" | Select -ExpandProperty DevicePath"

				//printf("%s\n", command);

				flush_and_call_system(command);

				//doesnt work
//				fread_s(drive, 3, 1, 3, stdin);

				//multiple instances cannot be run from the same working directory in this case
				//update this to use process id to make id unique

				//check for read stdout or other IPC
				//sprintf_s(tmp_file, )

				fopen_s(&newdrive, "newdrive", "r");

				fflush(newdrive);

				//very specific to single drive letter drives... rather than ole/wmi object path...
				//should replace this... esp for unicode systems in languages besides en-us
				fread(drive, 3, 1, newdrive);

				//null terminate string
				drive[1] = '\0';

				//printf("%s\n", drive);

				fclose(newdrive);
				
				remove("newdrive");

			}
			else
			{
				Py_RETURN_FALSE;
			}
		}
		else
		{
			PySys_WriteStdout("ISO File %s not found\n", filename);
			Py_RETURN_FALSE;
		}
	}

	//when using tp_obj->tp_free with pyobject_new there is no crash when calling help(pyIMAPI2FS)
	//though there is a crash on quit()
	//there is a crash if PyObject_Del is used in dealloc instead? 
	//py return none works here... narrowing down crashing code
	
	//this may be used for tp_new in pyIMAPI2FS_type, paramter 0 is object definition with all tp
	//members, NOT ob_type
	
	//printf("about to python new the obj");
	obj = imapi2fs_object_new(&pyIMAPI2FS_type, 0);

	store_filename(obj, filename);

	//needs to be updated to wchar_t for unicode
	strcpy_s(obj->tmp_filename, MAX_PATH*sizeof(char), write_filename);

	//copy drive letter to obj for ease of use in extract, list and similar functions
	memcpy(obj->drive, drive, 2);
	
	//printf("%s\n", obj->drive);
	
	obj->open = 1;
	obj->type = type;
	
	strcpy_s(obj->mode, 3, mode);

	obj->mode_str = PyString_FromString(obj->mode);
	
	memset(obj->path, 0, sizeof(obj->path));

	//printf("after set file handle\n");

	obj->IMAPI2FS_Object = fsi;
	
	obj->disk_type = PyString_FromString(disk_type);

	//printf("about to check for append mode to copy files\n");

	//for this "emulated" append mode
	if (!_strcmpi(obj->mode, "a")) 
	{
		char root[5];

		sprintf_s(root, 5, "%s:\\", obj->drive);

		//printf("adding existing files, %s\n", root);
		
		//add the files in the existing image to the new image 
		//(not a sector by sector dup or multisession)
		
		//add_existing_files(obj);

		fadd(obj->IMAPI2FS_Object, root, FALSE);

		//printf("added files\n");
	}

	return (PyObject *)obj;
}

/*
ex2_object
*/

static PyObject *imapi2fs_isFileSystem(PyObject *self, PyObject *args)
{
	PyObject *object = NULL;

	if (!PyArg_ParseTuple(args, "O", &object))
		return NULL;

	//typecheck against
	if(object_check(object))
		Py_RETURN_TRUE;
	
	Py_RETURN_FALSE;
}

//since c requires forward declare
//method table global
//python object methods and method table for module have similar structure and usage

static PyMethodDef module_name_methods[] = {
	//python function name, actual/export function name, calling convention (METH_VARARGS, METH_NOARGS, or METH_VARARGS|METH_KEYWORDS or rarely old obsolete 0)
	{ "open", (PyCFunction) imapi2fs_open, METH_VARARGS | METH_KEYWORDS, "Create or open a new IMAPI2 FileSystem object with the optionally provided filename=\"test.iso\", stdlib open mode='w',( or 'a', 'r'), and disk_type=\"CD\" (\"CD\", \"DVD\", \"DVDDL\",\"BluRay\")" },
	{ "is_FileSystem", imapi2fs_isFileSystem, METH_VARARGS, "Return True if object is a "module_name".FileSystem" },
	//NULL termination block (SENTINEL)
/*#if PY_MAJOR_VERSION >= 3
	{ NULL, NULL, 0, NULL }
#endif*/
	//another example has sentinel as
//#if PY_MAJOR_VERSION == 2
	{ NULL, NULL }
//#endif
};
static void imapi2fs_dealloc(PyObject *self)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	if (obj == NULL)
		return;


	//printf("deallocing obj");
	/*
	Now a Noddy object doesn't do very much and so doesn't need to implement many type methods.One you
	can't avoid is handling deallocation, so we find

		static void
		noddy_noddy_dealloc(PyObject* self)
	{
		PyObject_Del(self);
	}
	This is so short as to be self explanatory.This function will be called when the reference count on
	a Noddy object reaches 0 (or it is found as part of an unreachable cycle by the cyclic garbage 
	collector).PyObject_Del() is what you call when you want an object to go away.If a Noddy object
	held references to other Python objects, one would decref them here.
	*/

	//based on source in xxmodule and _collections
	//in case ex2_object_attr is null, using xdecref
	
	switch (obj->type)
	{
		//case imapi:
		case internal:

		if (obj->open)
		{
			PySys_WriteStdout("Image %s was still open... Unmounting or closing image...\n", obj->filename);
			imapi2fs_close(self, NULL);
		}

		if(_strcmpi(obj->mode, "r"))
		{
			//printf("imapi close\n");
			fdeleteIMAPI2FS(obj->IMAPI2FS_Object);
			//break;

		}

		break;

		case external:
		{
			//external_image_tool->unload();
			break;
		}
	}

	free(obj->filename);
	//Py_XDECREF(self->object_attr);
	//Py_XDECREF(self->file_handle);
	Py_XDECREF(obj->filename_str);
	
	//use fn ptr in struct to free
	self->ob_type->tp_free((PyObject*)self);

	/*
	example from documentaiton

	Py_XDECREF(self->first)
	PY_XDECREF(self->last)
	self->ob_type->tp_free((PyObject*)self);
	*/
}
//typedef int (*printfunc)(PyObject *, FILE *, int);
//example based on types from _collectionsmodule.c
static int imapi2fs_print(imapi2fs_object *self, FILE *fp, int flags)
{
	int status = 0;

	//called when object is "run" on the interactive interpreter... 
	//>>>test
	//IMAPI2FS FileSystem
	Py_BEGIN_ALLOW_THREADS
	fprintf(fp, "IMAPI2FS FileSystem: %s", self->filename);
	Py_END_ALLOW_THREADS
	return status;
}

static PyObject *imapi2fs_repr(imapi2fs_object *self)
{
	//can be called by test.__repr__()
	return PyString_FromFormat("IMAPI2 FileSystem: %s", self->filename);

}

static PyTypeObject pyIMAPI2FS_type = {
	/* The ob_type field must be initialized in the module init function
	* to be portable to Windows without using C++. */
	//PyVarObject_HEAD_INIT(NULL, 0)
	PyObject_HEAD_INIT(NULL)
#if PY_MAJOR_VERSION == 2
	0,							//tp_internal? tp_reserved?	//obj_size ob_size
	filesystem_type_name"."module_name, /*tp_name*/
#endif
#if PY_MAJOR_VERSION >= 3
	module_name"."filesystem_type_name,
#endif
	sizeof(imapi2fs_object),    /*tp_basicsize*/
	0,                          /*tp_itemsize*/
	/* methods */
	(destructor)imapi2fs_dealloc, /*tp_dealloc*/
	(printfunc)imapi2fs_print,    /*tp_print*/
	0,							/*tp_getattr - char * version referring to an attr */
	0,							/*tp_setattr - char * version */
	0,							/*tp_compare py2, tp_as_async py3*/
	(reprfunc)imapi2fs_repr,	/*tp_repr*/
	0,							/*tp_as_number*/
	0,							/*tp_as_sequence*/
	0,							/*tp_as_mapping*/
	0,							/*tp_hash*/
	0,							/*tp_call*/
	0,							/*tp_str*/
	0,							/*tp_getattro - PyObject * version referring to an attr */
	0,							/*tp_setattro - PyObject * version */
	0,							/*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,			/*tp_flags*/
	"pyIMAPI2 File System COM Object built on "__DATE__" at "__TIME__,/*tp_doc*/
	0,                      /*tp_traverse*/
	0,                      /*tp_clear*/
	0,                      /*tp_richcompare*/
	0,                      /*tp_weaklistoffset*/
	0,                      /*tp_iter*/
	0,                      /*tp_iternext*/
	/*
	If tp_methods is not NULL, it must refer to an array of PyMethodDef structures.
	Each entry in the table is an instance of this structure:

	typedef struct PyMethodDef {
	const char  *ml_name;       // method name
	PyCFunction  ml_meth;       // implementation function
	int          ml_flags;      // flags
	const char  *ml_doc;        // docstring
	} PyMethodDef;
	*/
	FileSystem_methods,     /*tp_methods*/
	0,                      /*tp_members*/
	0,                      /*tp_getset*/
	0,                      /*tp_base*/
	0,                      /*tp_dict*/
	0,                      /*tp_descr_get*/
	0,                      /*tp_descr_set*/
	0,                      /*tp_dictoffset*/
	//tp_init allows initialization of members to values other than NULL
	//i.e. parameterized ctor
	0,//(initproc),                      /*tp_init*/
	/*
	https://docs.python.org/2/extending/newtypes.html#adding-data-and-methods-to-the-basic-example

	We don�t fill the tp_alloc slot ourselves. Rather PyType_Ready() fills it for us by inheriting
	it from our base class, which is object by default. Most types use the default allocation.
	
	Still seems to require PyType_GenericAlloc
	*/
#if PY_MAJOR_VERSION == 2
	0,		/*tp_alloc for Py2, set by python*/
#endif
#if PY_MAJOR_VERSION >= 3
	PyType_GenericAlloc,	/*tp_alloc for Py3*/
#endif

	  //by replacing this line with "0," type instances have been prevented by invoking the ctor...
	//e.g. pyIMAPI2FS.FileSystem()
	(newfunc)imapi2fs_explain,//(newfunc)imapi2fs_object_new,//PyType_GenericNew,		/*tp_new*/
	//longobject uses PyObject_Del for free... 
	0,//PyObject_Del,//PyObject_GC_Del,        /*tp_free*/
	0,                      /*tp_is_gc*/
};

#if PY_MAJOR_VERSION >= 3

static int module_name_traverse(PyObject *m, visitproc visit, void *arg) {
	Py_VISIT(GETSTATE(m)->imapi2fs_error);
	Py_VISIT(GETSTATE(m)->imapi2fs);
	return 0;
}

static int module_name_clear(PyObject *m) {
	Py_CLEAR(GETSTATE(m)->imapi2fs_error);
	Py_CLEAR(GETSTATE(m)->imapi2fs);
	return 0;
}

//Py3 ModuleDef structure
static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	module_name,
	NULL,
	sizeof(struct module_state),
//not sure where the following -1 is originally from, found it in Knio/py35win64ext on github, havent tried yet (per-interpreter state is better anyway)
//	-1,      /* size of per-interpreter state of the module,
//			 or -1 if the module keeps state in global variables. */
	module_name_methods,
	NULL,
	module_name_traverse,
	module_name_clear,
	NULL
};

#endif

/*
from include/object.h

typedef void (*freefunc)(void *);
typedef void (*destructor)(PyObject *);
typedef int (*printfunc)(PyObject *, FILE *, int);
typedef PyObject *(*getattrfunc)(PyObject *, char *);
typedef PyObject *(*getattrofunc)(PyObject *, PyObject *);
typedef int (*setattrfunc)(PyObject *, char *, PyObject *);
typedef int (*setattrofunc)(PyObject *, PyObject *, PyObject *);
typedef int (*cmpfunc)(PyObject *, PyObject *);
typedef PyObject *(*reprfunc)(PyObject *);
typedef long (*hashfunc)(PyObject *);
typedef PyObject *(*richcmpfunc) (PyObject *, PyObject *, int);
typedef PyObject *(*getiterfunc) (PyObject *);
typedef PyObject *(*iternextfunc) (PyObject *);
typedef PyObject *(*descrgetfunc) (PyObject *, PyObject *, PyObject *);
typedef int (*descrsetfunc) (PyObject *, PyObject *, PyObject *);
typedef int (*initproc)(PyObject *, PyObject *, PyObject *);
typedef PyObject *(*newfunc)(struct _typeobject *, PyObject *, PyObject *);
typedef PyObject *(*allocfunc)(struct _typeobject *, Py_ssize_t);

*/



//only function in module.def/exports that is not static, #define is either void for C or extern "C" for C++


//match return and init fn name
#if PY_MAJOR_VERSION >= 3
	#define INITERROR return NULL
//return signature defined for both 2 and 3
PyMODINIT_FUNC PyInit_pyIMAPI(void)
#endif
#if PY_MAJOR_VERSION == 2
#define INITERROR return
	//return signature defined for both 2 and 3
PyMODINIT_FUNC initpyIMAPI(void)
#endif
{
	//declare module instance pointer
	PyObject *module_instance;

	struct module_state *st;

//#define __DEBUGBREAK__
#ifdef __DEBUGBREAK__
	DebugBreak();
#endif

#if PY_MAJOR_VERSION >= 3
	module_instance = PyModule_Create(&moduledef);
#endif
#if PY_MAJOR_VERSION == 2
	
	
	//initialize the module instance, original documentation specified py_InitModule (there are also py_InitModule2, 3, 4 functions) 
	module_instance = Py_InitModule3(module_name, module_name_methods, module_doc);
	
#endif


	//if the module could not be initialized, the return value was NULL/nullptr
	if (module_instance == NULL)
	{
		INITERROR;
	}

	st = GETSTATE(module_instance);

	//name is the class name returned with type(), req python dotted name format (wrong style of name can cause a python crash), e.g. no underscore!
	st->imapi2fs_error = PyErr_NewException(error_type_name, NULL, NULL);

#if PY_MAJOR_VERSION == 2
	
	Py_INCREF(st->imapi2fs_error);

	//name is the name used to create and instance of the object, i.e. pythonex2.error()
	PyModule_AddObject(module_instance, error_name, st->imapi2fs_error);
	

#endif

	//must be ready'd before use ( methods arent found and help crashes (at least in 3.6.5) et c if this is not called )
	if (PyType_Ready(&pyIMAPI2FS_type) < 0)
	{
		//at least log or print error!
		//DebugBreak();
		INITERROR;
	}

	//in MSVC the TypeObject def macro is not passed this (must be constants), so it should be set manually here
	//while
	//pyIMAPI2FS_type.ob_type = &PyType_Type;
	//works in 2.7, it does not compile with the 3.6 source, the following replaces it for both
	Py_TYPE(&pyIMAPI2FS_type) = &PyType_Type;

	//inc ref count on ?module?
	/*&st->imapi2fs runs, but st->imapi2fs crashes*/
	Py_INCREF(&st->imapi2fs);

	//printf("imapi2fs->tp_alloc %p", pyIMAPI2FS_type.tp_alloc);
	//printf("imapi2fs->tp_methods %p", pyIMAPI2FS_type.tp_methods);

	//inc ref count on filesystem type
	Py_INCREF(&pyIMAPI2FS_type);

	PyModule_AddObject(module_instance, filesystem_type_name, (PyObject *)&pyIMAPI2FS_type);

//#endif
	/*
	int j = sizeof(sz_disk_types) / sizeof(char *);

	for (int i = 0; i < j; i++)
	{
		PyModule_AddObject(module_instance, sz_disk_types[i], Py_BuildValue("I", imapi_disk_types[i]));
	}
	*/
	
	//PyObject_Print()
	//Py_DECREF();
	//ex1_object = PyObject_New()
	/*
	PyRun_SimpleString(	"import platform\n"
						"print platform.version()");
	*/

	//if building locally should detect if target machine is pre-win8.1, though still win8 (powershell w/ mount)
	//and use getversion or always use external (e.g. 7zip)

	//GetVersion()
	
#ifdef IsWindows8orGreater
	//pyd build doesnt seem to find these anywhere (vc for python? add sdk path env vars to distutils extension?)
	if (!IsWindows8orGreater())
	{
		//IsWindows7orGreater()
		//check reg key for powershell
		//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\PowerShell\1

		PySys_WriteStdout("Powershell support for Mount-DiskImage is not available. 7z or another external library may be used.");

		external_dll = LoadLibraryA("7za.dll");

		PySys_WriteStdout("Using external minimal 7-zip ISO dll - 7-zip ISO support is LGPL - 7-Zip Copyright (C) 1999-2016 Igor Pavlov. 7-Zip Source can be obtained from www.7-zip.org");

		no_mount = 1;
	}
#endif

#if PY_MAJOR_VERSION >= 3

	//Python3 init<ModuleName> function signature includes return of the PyModule_Create instantiated object
	return module_instance;

#endif

}