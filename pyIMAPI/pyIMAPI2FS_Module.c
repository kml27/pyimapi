//Kenneth Long
//9.17.2016
//IMAPI2 File System Python Module

#include <windows.h>
#include <Shlwapi.h>
#include <Python.h>
#include "pyIMAPI.h"

#pragma comment(lib, "shlwapi.lib")

#define error_name						"IMAPI2Error"
#define module_name						"pyIMAPI"
#define error_type_name					module_name"."error_name

#define filesystem_type_name			"FileSystem"

staticforward PyTypeObject pyIMAPI2FS_type;

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

static PyObject *imapi2fs;
static PyObject *imapi2fs_error;

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
	imapi,		//creating an iso image in xp sp2+
	mount,		//using powershell in win8+ to read the file
	external,	//if 7z or alternative is used to work with iso files
};

typedef enum _objType objType;

typedef struct {
	PyObject_HEAD
		PyObject		*filename_str;
		char			*filename;
		void			*IMAPI2FS_Object;
		objType			type;
		//use unicode and prepend "\\?\" to exceed max_path on newer windows before 10, 10 doesnt require it...
		char			drive[2];
		char			path[MAX_PATH];
		int				open;
		PyObject		*disk_type;
} imapi2fs_object;

static imapi2fs_object *get_imapi2fs(PyObject *self)
{
	if (!object_check(self))
		return NULL;

	return (imapi2fs_object *)self;
}

//memberdef
//method name, type, offset, 0, description
static PyObject *imapi2fs_add(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *result = "";

	char *filename = NULL;

	if (obj->type == mount)
	{
		PySys_WriteStdout("Mounted filesystem is read-only, cannot modiy image\n");
		Py_RETURN_FALSE;
	}
	//printf("add");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	if (result = fadd(obj->IMAPI2FS_Object, filename))
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
	
	if (obj->type == mount)
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
		case imapi:
		{
			if (fchdir(obj->IMAPI2FS_Object, filename) == (void*)NULL)
				Py_RETURN_TRUE;

			break;
		}

		case mount:
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
				
			break;
		}
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
		case imapi:
		{
			cwd = fgetcwd(obj->IMAPI2FS_Object);

			ret = PyString_FromString(cwd);

			ffreecwd(cwd);

			if (ret == NULL)
				Py_RETURN_NONE;
			break;
		}
		case mount:
		{
			if (strlen(obj->path))
				ret = PyString_FromString(obj->path);
			else
				ret = PyString_FromString("\\");
			break;
		}
		case external:
		{
			break;
		}
	}
	//printf("building pyvalue");
	return ret;
}

static PyObject *imapi2fs_list(PyObject *self, PyObject *args, PyObject *keywds)
{
	
	imapi2fs_object *obj = get_imapi2fs(self);

	static char *kwlist[] = { "verbose", NULL };

	int number_files=0;
	char **paths;
	int count = 0;
	int i=0;
	//get number of files

	char verbose = 1;
	PyObject *list = NULL, *item = NULL;

	//printf("entering list()");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|b:keywords", kwlist, &verbose))
		return NULL;

	switch (obj->type)
	{
		case imapi:
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

			break;
		}
		
		case mount:
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
				printf("no files\n");
			}

			break;
		}
		
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
		return NULL;
	}

	//printf("%s %s\n", isofile_member, system_path);

	//if a system path was supplied, but does not exist

	/*if (system_path && !PathFileExistsA(system_path))
	{
		CreateDirectoryA(system_path, NULL);
	}*/

	switch (obj->type)
	{
		case imapi:
		{
			PySys_WriteStdout("Mounted filesystem is write-only, cannot read image\n");
			Py_RETURN_FALSE;
			break;
		}

		case mount:
		{
			if (!strcmp(system_path, ""))
			{
				system_path = ".";
			}

			//if (!fextract(obj->IMAPI2FS_Object, isofile_member, system_path))
			//	Py_RETURN_FALSE;

			sprintf_s(copy_file, MAX_CLL, "copy \"%s:\\%s\" \"%s\" > nul", obj->drive, isofile_member, system_path);

			//printf("%s\n", copy_file);

			system(copy_file);

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
		case imapi:
		{
			if (fexists(obj->IMAPI2FS_Object, filename))
			{
				Py_RETURN_TRUE;
			}
			break;
		}

		case mount:
		{
			struct stat info;
			char fullpath[MAX_PATH];
			sprintf_s(fullpath, MAX_PATH, "%s:\\%s\\%s", obj->drive, obj->path, filename);
			//printf("%s\n", fullpath);
			
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
	//printf("close");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!obj->open)
	{
		//should this throw an exception?
		Py_RETURN_FALSE;
	}

	switch (obj->type)
	{
		case imapi:
			if (obj != NULL)
			{
				//printf("calling dll to close image");
				fcloseImage(obj->IMAPI2FS_Object);
			}
			break;
		case mount:
		{
			char dismount_ps[MAX_CLL];
		
			//printf("%s\n", obj->filename);
			sprintf_s(dismount_ps, MAX_CLL, "powershell -Command \"Dismount-DiskImage -Image \"%s\"\"", obj->filename);
			
			system(dismount_ps);
			
			break;
		}
		case external:
			break;
	}

	//printf("returning");

	obj->open = 0;

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
	
	if (obj->type == mount)
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

//methods array with which to set tp_methods struct member
static PyMethodDef FileSystem_methods[] =	{	//name, function, args (METH_NOARGS, METH_VARARGS, METH_VARARGS|METH_KEYWORDS), description
												{"add", (PyCFunction)imapi2fs_add,  METH_VARARGS, PyDoc_STR("add a file to the ISO filesystem") },
												{"mkdir", (PyCFunction)imapi2fs_mkdir, METH_VARARGS, PyDoc_STR("make a directory in the ISO filesystem") },
												{"chdir", (PyCFunction)imapi2fs_chdir, METH_VARARGS, PyDoc_STR("change current working directory in ISO filesystem")},
												{"getcwd", (PyCFunction)imapi2fs_getcwd, METH_NOARGS, PyDoc_STR("return current wording directory in ISO filesystem")},
												{"list", (PyCFunction)imapi2fs_list, METH_KEYWORDS, PyDoc_STR("return list [] of files in ISO filesystem")},
												{"extract", (PyCFunction)imapi2fs_extract, METH_KEYWORDS, PyDoc_STR("extract a file from the ISO filesystem") },
												{"exists", (PyCFunction)imapi2fs_exists, METH_VARARGS, PyDoc_STR("check if the specified file exists in the ISO filesystem")},
												{"close", (PyCFunction)imapi2fs_close, METH_NOARGS, PyDoc_STR("close this ISO filesystem")},
												{"remove", (PyCFunction)imapi2fs_remove, METH_VARARGS, PyDoc_STR("remove the specified file from the ISO filesystem")},
												{"getdisktype", (PyCFunction)imapi2fs_getdisktype, METH_NOARGS, PyDoc_STR("Return the string representation of specified physical media type")},
												{NULL}//, NULL }//SENTINEL *ml_name must be NULL 
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

	//allocate a new object, referencing static object instance for typechecking
	self = (imapi2fs_object *) type->tp_alloc(type, 0);

	// NO INCREF DECREF 

	//did the allocation succeed
	if (self == NULL)
	{
		Py_DECREF(self);
		return NULL;
	}
	
	self->IMAPI2FS_Object = NULL;

	memset(self->drive, '\0', 2);
	
	self->filename = NULL;
	self->open = 0;
	self->type = imapi;

	return self;
}

static PyObject *imapi2fs_open(PyObject *self, PyObject *args, PyObject *keywds)
{
	//https://docs.python.org/2.7/extending/extending.html#keyword-parameters-for-extension-functions
	//time_t ctime;
	//time(&ctime);

	char *filename = "default.iso";//asctime(localtime(&ctime));
	char *mode = "a";
	FILE *handle = NULL;
	void *fsi = NULL;

	imapi2fs_object *obj = NULL;
	PyObject *str = NULL;

	static char *kwlist[] = {"filename", "mode", "disk_type", NULL};

	objType type = imapi;

	char *disk_type = "CD";

	char *mount_drive = "z:\\";

	char path[MAX_PATH];
	char command[MAX_CLL];
	char fullpath[MAX_CLL];
	char drive[2];
	FILE *newdrive;

	//printf("parsing keywords\n");
	//:open
	//:new_noddy
	/*args should be filename and file open mode*/
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|sss:keywords", kwlist, &filename, &mode, &disk_type))
	{
		return NULL;
	}
	
	//printf("finished parsing keywords\n");
	
	if (!strcmp(mode, "a") || !strcmp(mode, "w"))
	{
		//printf("about to create internal object");
		fsi = fcreateIMAPI2FS(filename, mode, disk_type);
		//printf("%d", type);
		//printf("self is %p\n", self);
	}
	else if (!strcmp(mode, "r"))
	{
		type = mount;
		//if using windows 8 or later (or powershell is installed)
		//disks may be read by using powershell to mount the disk image

		GetCurrentDirectoryA((DWORD)MAX_PATH, path);

		sprintf_s(fullpath, MAX_PATH, "%s\\%s", path, filename);
		filename = fullpath;
		//printf("%s", filename);

//CHECK FOR fileexists here for (better?) robustness
		if (PathFileExistsA(filename)) {

			sprintf_s(command, MAX_CLL, "powershell -Command \"Mount-DiskImage -Image \"%s\"\"", filename);
			//printf("%s : %s",command);

			//ShellExecute(NULL, NULL, "powershell", mount_ps, NULL, )
			system(command);

			sprintf_s(command, MAX_CLL, "powershell -Command \"Get-DiskImage -ImagePath \"%s\" | Get-Volume | Select -ExpandProperty DriveLetter\" > newdrive", filename);

			//could also use DevicePath for more robust handling when implementing wcs/unicode
			//powershell -Command "Get-DiskImage -ImagePath "c:\development\pyimapi2\pyimapi\default.iso" | Select -ExpandProperty DevicePath"

			//printf("%s\n", command);

			system(command);

			//multiple instances cannot be run from the same working directory in this case
			//update this to use process id to make id unique
			fopen_s(&newdrive, "newdrive", "r");

			//very specific to single drive letter drives... rather than ole/wmi object path...
			//should replace this... esp for unicode systems in languages besides en-us
			fread(drive, 3, 1, newdrive);
			//printf("%s\n", drive);
			fclose(newdrive);
			//remove("newdrive");

			//null terminate string
			drive[1] = '\0';
			//printf("%s\n", drive);
		}
		else
		{
			PySys_WriteStdout("ISO File not found");
		}
	}

	//when using tp_obj->tp_free with pyobject_new there is no crash when calling help(pyIMAPI2FS)
	//though there is a crash on quit()
	//there is a crash if PyObject_Del is used in dealloc instead? 
	//py return none works here... narrowing down crashing code
	
	//this may be used for tp_new in pyIMAPI2FS_type, paramter 0 is object definition with all tp
	//members, NOT ob_type
	obj = imapi2fs_object_new(&pyIMAPI2FS_type, 0);

	//copy drive letter to obj for ease of use in extract, list and similar functions
	memcpy(obj->drive, drive, 2);
	//printf("%s\n", obj->drive);
	obj->open = 1;
	obj->type = type;
	memset(obj->path, 0, sizeof(obj->path));
	
	//printf("after set file handle\n");
	obj->IMAPI2FS_Object = fsi;
	//printf("after set internal obj instance\n");

	obj->filename = malloc(strlen(filename) + 1);
	memcpy(obj->filename, filename, strlen(filename)+1);
	
	str = PyString_FromString(filename);

	obj->disk_type = PyString_FromString(disk_type);

	//inc ref pyobjects
	obj->filename_str = str;

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

static PyMethodDef IMAPI2FS_methods[] = {
	//python function name, actual/export function name, calling convention (METH_VARARGS, METH_NOARGS, or METH_VARARGS|METH_KEYWORDS or rarely old obsolete 0)
	{ "open", (PyCFunction)imapi2fs_open, METH_KEYWORDS, "Create or open a new IMAPI2 FileSystem object with the optionally provided filename ('<time>'), stdlib open mode ('w'), and disk_type (\"CD\", \"DVD\", \"HDDVD\", \"DVDDL\",\"BluRay\")" },
	{ "is_FileSystem", imapi2fs_isFileSystem, METH_VARARGS, "Return true if name is an "module_name".FileSystem that this module can " },
	//NULL termination block (SENTINEL)
	//{ NULL, NULL, 0, NULL }
	//another example has sentinel as
	{ NULL, NULL }
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
		case imapi:
		{
			//printf("imapi close\n");
			fdeleteIMAPI2FS(obj->IMAPI2FS_Object);
			break;
		}

		case mount:
		{
			if (obj->open)
			{
				PySys_WriteStdout("Image %s was still open... Unmounting image...\n", obj->filename);
				imapi2fs_close(self, NULL);
			}
			break;
		}

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
	0,							//obj_size
	filesystem_type_name"."module_name, /*tp_name*/
	sizeof(imapi2fs_object),    /*tp_basicsize*/
	0,                          /*tp_itemsize*/
	/* methods */
	(destructor)imapi2fs_dealloc, /*tp_dealloc*/
	(printfunc)imapi2fs_print,    /*tp_print*/
	0,							/*tp_getattr - char * version referring to an attr */
	0,							/*tp_setattr - char * version */
	0,							/*tp_compare*/
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

	We don’t fill the tp_alloc slot ourselves. Rather PyType_Ready() fills it for us by inheriting
	it from our base class, which is object by default. Most types use the default allocation.
	
	Still seems to require PyType_GenericAlloc
	*/
	0,//PyType_GenericAlloc,	/*tp_alloc*/
	//by replacing this line with "0," type instances have been prevented by invoking the ctor...
	//e.g. pyIMAPI2FS.FileSystem()
	(newfunc)imapi2fs_explain,//(newfunc)imapi2fs_object_new,//PyType_GenericNew,		/*tp_new*/
	//longobject uses PyObject_Del for free... 
	0,//PyObject_Del,//PyObject_GC_Del,        /*tp_free*/
	0,                      /*tp_is_gc*/
};

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
PyMODINIT_FUNC
//documentation specified init<module_name>( void )
initpyIMAPI(void)
{
	//declare module instance pointer
	PyObject *module_instance;
//#define __DEBUGBREAK__
#ifdef __DEBUGBREAK__
	DebugBreak();
#endif
	pyIMAPI2FS_type.ob_type = &PyType_Type;

	//initialize the module instance, original documentation specified py_InitModule (there are also py_InitModule2, 3, 4 functions) 
	module_instance = Py_InitModule3(module_name, IMAPI2FS_methods, module_doc);

	//if the module could not be initialized, the return value was NULL/nullptr
	if (module_instance == NULL)
		return;

	//name is the class name returned with type(), req python dotted name format (wrong style of name can cause a python crash), e.g. no underscore!
	imapi2fs_error = PyErr_NewException(error_type_name, NULL, NULL);
	Py_INCREF(imapi2fs_error);

	//name is the name used to create and instance of the object, i.e. pythonex2.error()
	PyModule_AddObject(module_instance, error_name, imapi2fs_error);
	
	if (PyType_Ready(&pyIMAPI2FS_type) < 0)
	{
		//at least log or print error!
		//DebugBreak();
		return;
	}

	//inc ref count on ?module?
	Py_INCREF(&imapi2fs);

	//inc ref count on filesystem type
	Py_INCREF(&pyIMAPI2FS_type);

	PyModule_AddObject(module_instance, filesystem_type_name, (PyObject *)&pyIMAPI2FS_type);

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


}
