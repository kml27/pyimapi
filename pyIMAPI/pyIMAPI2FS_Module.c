//Kenneth Long
//9.17.2016
//IMAPI2 File System Python Module

#include <Python.h>
#include "pyIMAPI.h"
#include <windows.h>

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

//to determine if the object is of this object type
static PyTypeObject imapi2fs_type_object;

//test if object is of ex2 object type
#define object_check(t) (Py_TYPE(t) == &pyIMAPI2FS_type)

typedef struct {
	PyObject_HEAD
		//PyObject		*object_attr;
		PyObject		*filename_str;
		char			*filename;
		FILE			*file_handle;
		void			*IMAPI2FS_Object;
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

	char *filename = NULL;

	//printf("add");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	fadd(obj->IMAPI2FS_Object, filename);

	Py_RETURN_NONE;
}

static PyObject *imapi2fs_mkdir(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *filename = NULL;

	//printf("mkdir");

	if (obj == NULL)
		Py_RETURN_NONE;

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

	fchdir(obj->IMAPI2FS_Object, filename);

	Py_RETURN_NONE;
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

	cwd = fgetcwd(obj->IMAPI2FS_Object);
	
	ret = PyString_FromString(cwd);

	ffreecwd(cwd);

	if (ret == NULL)
		Py_RETURN_NONE;

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

	//printf("calling count");
	count = fcount(obj->IMAPI2FS_Object);
	//printf("calling list\n");
	paths = flist(obj->IMAPI2FS_Object);

	//get each full file path
	//printf("buidling python list\n");

	list = PyList_New(count);

	if (count > 0 && list!=NULL)
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

	return list;
}

static PyObject *imapi2fs_extract(PyObject *self, PyObject *args, PyObject *keywds)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	static char *kwlist[] = {"member","path",NULL};

	char *member = "";
	char *path = "";

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|s:keywords", kwlist, &member, &path))
		return NULL;

	//get file item in iso
	//open local file handle
	//write datastream to file
	//close file

	Py_RETURN_NONE;
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

	if(fexists(obj->IMAPI2FS_Object, filename))
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject *imapi2fs_close(PyObject *self, PyObject *noarg)
{
	imapi2fs_object *obj = get_imapi2fs(self);
	//printf("close");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (obj != NULL)
	{
		//printf("calling dll to close image");
		fcloseImage(obj->IMAPI2FS_Object);
	}

	//printf("returning");

	Py_RETURN_NONE;
}
static PyObject *imapi2fs_remove(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj = get_imapi2fs(self);

	char *filename = NULL;
	int result=0;
	//printf("remove");

	if (obj == NULL)
		Py_RETURN_NONE;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;

	//result = 
		fremove(obj->IMAPI2FS_Object, filename);

	if(result)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;

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
												{NULL}//, NULL }//SENTINEL *ml_name must be NULL 
											};

//static PySequenceMethods FileSystem_sequence_methods[]={}
static imapi2fs_object *imapi2fs_explain(PyTypeObject *type, PyObject *arg)
{
	printf("FileSystem objects cannot be instantiated directly, use "module_name".open()");
	return NULL;
}

//replacing with only use open
static imapi2fs_object *imapi2fs_object_new(PyTypeObject *type, PyObject *arg)
{	//ptr to modules object type
	imapi2fs_object *self;

	//printf("using object_new\n");
	//printf("type object matches %i", type == &imapi2fs_type_object);
	//DebugBreak();
	//allocate a new object, referencing static object instance for typechecking(?)
	self = (imapi2fs_object *) type->tp_alloc(type, 0);//PyObject_New(imapi2fs_object, &imapi2fs_type_object);

	// NO INCREF DECREF 

	//did the allocation succeed
	if (self == NULL)
	{
		Py_DECREF(self);
		return NULL;
	}
	//set attr to NULL
	//self->object_attr = NULL;
	//self->filename_str = PyString_FromString("");
	self->file_handle = NULL;
	self->IMAPI2FS_Object = NULL;
	return self;
}

/******************************************************************************************************/
/*out of place, required unless a prototype defined extern and supplied after object (for code organization)*/
/*
static PyObject *imapi2fs_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	imapi2fs_object *obj;

	if (!PyArg_ParseTuple(args, ":new"))
		return NULL;

	obj = imapi2fs_object_new(type, args);

	if (obj == NULL)
		return NULL;

	return (PyObject *)obj;
}
*/


static PyObject *imapi2fs_open(PyObject *self, PyObject *args, PyObject *keywds)
{
	//https://docs.python.org/2.7/extending/extending.html#keyword-parameters-for-extension-functions
	//time_t ctime;
	//time(&ctime);

	const char *filename = "default.iso";//asctime(localtime(&ctime));
	char *mode = "a";
	FILE *handle = NULL;
	void *fsi = NULL;

	//PyObject *filenameArg, *modeArg;

	imapi2fs_object *obj = NULL;
	PyObject *str = NULL;

	static char *kwlist[] = {"filename", "mode", NULL};

	//printf("parsing keywords\n");
	//:open
	//:new_noddy
	/*args should be filename and file open mode*/
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "|ss:keywords", kwlist, &filename, &mode))
	{
		return NULL;
	}
	
	//printf("finished parsing keywords\n");
	
	
	/*Py_BEGIN_ALLOW_THREADS
		printf(filename);
		printf(mode);//fprintf(fp, "ex2 object!", 0);
		printf("\n");
	Py_END_ALLOW_THREADS
	*/

	//should have IMAPI2 FS object create this file... here as a placeholder for now
	/*handle = fopen(filename, mode);

	if (handle == NULL)
	{
		printf("error opening file");
		//create error if error opening file
	}*/
	
	//printf("about to create internal object");
	fsi = fcreateIMAPI2FS(filename, mode);
	//printf("%d", type);
	//printf("self is %p\n", self);
	
	//when using tp_obj->tp_free with pyobject_new there is no crash when calling help(pyIMAPI2FS)
	//though there is a crash on quit()
	//there is a crash if PyObject_Del is used in dealloc instead? 
	//py return none works here... narrowing down crashing code
	
	//this may be used for tp_new in pyIMAPI2FS_type, paramter 0 is object definition with all tp
	//members, NOT ob_type
	obj = imapi2fs_object_new(&pyIMAPI2FS_type, 0);
	
	//obj->file_handle = handle;
	//printf("after set file handle\n");
	obj->IMAPI2FS_Object = fsi;
	//printf("after set internal obj instance\n");
	obj->filename = malloc(strlen(filename) + 1);
	memcpy(obj->filename, filename, strlen(filename));
	
	str = PyString_FromString(filename);

	//inc ref pyobjects
	//Py_INCREF(str);
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
	{ "open", (PyCFunction)imapi2fs_open, METH_KEYWORDS, "Create or open a new IMAPI2 FileSystem object with the optionally provided filename ('<time>') and stdlib open mode ('a+')" },
	{ "is_FileSystem", imapi2fs_isFileSystem, METH_VARARGS, "Return true if name is an "module_name".FileSystem that this module can " },
	//NULL termination block (SENTINEL)
	//{ NULL, NULL, 0, NULL }
	//another example has sentinel as
	{ NULL, NULL }
};
static void imapi2fs_dealloc(imapi2fs_object *self)
{
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
	//printf("obj to del is at %p\n", self);
	//printf("deleting internal object\n");
	/*if (self != NULL)
		printf("internal obj at %p\n", self->IMAPI2FS_Object);
	*/

	fdeleteIMAPI2FS(self->IMAPI2FS_Object);
	
	free(self->filename);
	//Py_XDECREF(self->object_attr);
	//Py_XDECREF(self->file_handle);
	Py_XDECREF(self->filename_str);
	//printf("closing file handle %d\n", self->file_handle);
	
	/*if(self->file_handle!=NULL)
		fclose(self->file_handle);
	*/

	//printf("using self ob_type %p"/* free %p"*/, self->ob_type);//, (self->ob_type != NULL ? self->ob_type->tp_free : self->ob_type));
	self->ob_type->tp_free((PyObject*)self);

	//PyObject_Del(self);
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
	return PyString_FromFormat("IMAPI2 FileSystem: %p", self);//, self->filename_str);
}
/*
static PyObject *imapi2fs_getattr(imapi2fs_object *self, char *name)
{
	printf("getattr char* called");
	printf("getting attribute ");
	printf(name);
	printf("\n");

	if (self->object_attr != NULL)
	{
		PyObject *attr = PyDict_GetItemString(self->object_attr, name);
		if (attr != NULL)
		{
			Py_INCREF(attr);
			return attr;
		}
	}

	printf("trying py_findmethod\n");

	//cast due to the call originally being of the modules object type
	return Py_FindMethod(IMAPI2FS_methods, (PyObject *)self, name);
}

static PyObject *imapi2fs_setattr(imapi2fs_object *self, char *name, PyObject *obj)
{
	printf("set attr char* called");
	if (self->object_attr == NULL)
	{
		//allocate attr
		self->object_attr = PyDict_New();

		//if alloc failed, return -1 as error
		if (self->object_attr == NULL)
			return NULL;
	}

	if (obj == NULL)
	{
		//del attr
		int status = PyDict_DelItemString(self->object_attr, name);
		if (status < 0)
			PyErr_SetString(PyExc_AttributeError, "attempt to delete non-existent attribute");

		//return status;
		return NULL;
	}
	else
		return PyDict_SetItemString(self->object_attr, name, obj);
}
*/


static PyTypeObject pyIMAPI2FS_type = {
	/* The ob_type field must be initialized in the module init function
	* to be portable to Windows without using C++. */
	//PyVarObject_HEAD_INIT(NULL, 0)
	PyObject_HEAD_INIT(NULL)
	0,//obj_size
	filesystem_type_name"."module_name,
	//"pyIMAPI2FS.FileSystem",             /*tp_name*/
	sizeof(imapi2fs_object),          /*tp_basicsize*/
	0,                          /*tp_itemsize*/
	/* methods */
	(destructor)imapi2fs_dealloc, /*tp_dealloc*/
	(printfunc)imapi2fs_print,    /*tp_print*/
	0,//(getattrfunc)imapi2fs_getattr, /*tp_getattr - char * version referring to an attr */
	0,//(setattrfunc)imapi2fs_setattr, /*tp_setattr - char * version */
	0,                          /*tp_compare*/
	(reprfunc)imapi2fs_repr,    /*tp_repr*/
	0,                          /*tp_as_number*/
	0,                          /*tp_as_sequence*/
	0,                          /*tp_as_mapping*/
	0,                          /*tp_hash*/
	0,                      /*tp_call*/
	0,                      /*tp_str*/
	0,                      /*tp_getattro - PyObject * version referring to an attr */
	0,                      /*tp_setattro - PyObject * version */
	0,                      /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,     /*tp_flags*/
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
}
