//Kenneth Long
//9.17.2016
//IMAPI2 File System Python Module

#include <Python.h>
#include "pyIMAPI.h"


PyDoc_STRVAR(module_doc, "Windows IMAPI2 File System Python Module\n\
				FileSystem: an IMAPI2 FileSystem\n\
				Error:		a catch all exception for any errors\n");

/*
maybe
FileSystem:	`` with control over underlying structure
DataCD:		an IMAPI2 FileSystem based object\n\
AudioCD:	an IMAPI2 Raw Format DiscAtOnce compatible object\n\
*/

#define error_name						"IMAPI2Error"
#define module_name						"pyIMAPI2FS"
#define error_type_name					"pyIMAPI2FS.IMAPI2Error"

/*
#define datacd_filesystem_type_name		"DataCD"
#define audiocd_filesystem_type_name	"AudioCD"
*/

#define filesystem_type_name			"FileSystem"

static PyObject *imapi2fs;
static PyObject *imapi2fs_error;

typedef struct {
	PyObject_HEAD
		PyObject		*object_attr;
		PyObject		*filename_str;
		FILE			*file_handle;
		void			*IMAPI2FS_Object;
		int				int_val;
} imapi2fs_object;

//memberdef
//method name, type, offset, 0, description
static PyObject *imapi2fs_add(imapi2fs_object *self, PyObject *args)
{
	char *filename = NULL;

	PyArg_ParseTuple(args, "s", &filename);

	fadd(self->IMAPI2FS_Object, filename);

	Py_RETURN_NONE;
}

static PyObject *imapi2fs_mkdir(imapi2fs_object *self)
{
	Py_RETURN_NONE;
}
static PyObject *imapi2fs_chdir(imapi2fs_object *self)
{
	Py_RETURN_NONE;
}

static PyObject *imapi2fs_getcwd(imapi2fs_object *self)
{
	char *cwd = fgetcwd(self->IMAPI2FS_Object);

	return Py_BuildValue("s", cwd);
}

static PyObject *imapi2fs_list(imapi2fs_object *self, PyObject *args)
{
	int number_files=0;
	char *value_format_specifier;
	char **paths;
	int count = 0;
	PyObject *retval;
	int i=0;
	//get number of files

	count = fcount(self->IMAPI2FS_Object);

	paths = flist(self->IMAPI2FS_Object);

	//get each full file path
	value_format_specifier = malloc(count * 2+2);
	
	for (i = 0; i < count; i++)
	{
		value_format_specifier[i*2] = 's';
		
		if(i < count-1)
			value_format_specifier[(i * 2) + 1] = ',';
	}

	retval = Py_BuildValue(value_format_specifier, paths);
	free(value_format_specifier);
	ffreelist(self->IMAPI2FS_Object, paths);

	return retval;
}

static PyObject *imapi2fs_extract(imapi2fs_object *self)
{
	Py_RETURN_NONE;
}

static PyObject *imapi2fs_exists(imapi2fs_object *self, PyObject *args)
{
	char *filename = NULL;

	PyArg_ParseTuple(args, "s", &filename);

	if(fexists(self, filename))
	{
		Py_RETURN_TRUE;
	}

	Py_RETURN_FALSE;
}

static PyObject *imapi2fs_close(imapi2fs_object *self)
{
	fimageclose(self);

	Py_RETURN_NONE;
}
static PyObject *imapi2fs_remove(imapi2fs_object *self, PyObject *args)
{
	char *filename = NULL;
	int result=0;

	PyArg_ParseTuple(args, "s", &filename);

	//result = 
		fremove(self->IMAPI2FS_Object, filename);

	if(result)
		Py_RETURN_TRUE;
	else
		Py_RETURN_FALSE;

}

//name should match that of filesystem_type_name
static PyMethodDef FileSystem_methods[] =	{	//name, function, args (METH_NOARGS, METH_VARARGS, METH_VARARGS|METH_KEYWORDS), description
												{"add", (PyCFunction)imapi2fs_add,  METH_VARARGS, "add a file to the ISO filesystem" },
												{"mkdir", (PyCFunction)imapi2fs_mkdir, METH_VARARGS, "make a directory in the ISO filesystem" },
												{"chdir", (PyCFunction)imapi2fs_chdir, METH_VARARGS, "change current working directory in ISO filesystem"},
												{"getcwd", (PyCFunction)imapi2fs_getcwd, METH_NOARGS, "return current wording directory in ISO filesystem"},
												{"list", (PyCFunction)imapi2fs_list, METH_NOARGS, "return list [] of files in ISO filesystem"},
												{"extract", (PyCFunction)imapi2fs_extract, METH_VARARGS|METH_KEYWORDS, "extract a file from the ISO filesystem" },
												{"exists", (PyCFunction)imapi2fs_exists, METH_VARARGS, "check if the specified file exists in the ISO filesystem"},
												{"close", (PyCFunction)imapi2fs_close, METH_NOARGS, "close this ISO filesystem"},
												{"remove", (PyCFunction)imapi2fs_remove, METH_VARARGS, "remove the specified file from the ISO filesystem"}
											};

//static PySequenceMethods FileSystem_sequence_methods[]={}

//to determine if the object is of this object type
static PyTypeObject imapi2fs_type_object;

//test if object is of ex2 object type
#define object_check(t) (Py_TYPE(t) == &imapi2fs_type_object)


static imapi2fs_object *newimapi2fs_object(PyObject *arg)
{
	//ptr to modules object type
	imapi2fs_object *self;

	//allocate a new object, referencing static object instance for typechecking(?)
	self = PyObject_New(imapi2fs_object, &imapi2fs_type_object);

	// NO INCREF DECREF 

	//did the allocation succeed
	if (self == NULL)
		return NULL;

	//set attr to NULL
	self->object_attr = NULL;

	return self;
}


static PyObject *imapi2fs_open(PyObject *self, PyObject *args, PyObject *keywds)
{
	//https://docs.python.org/2.7/extending/extending.html#keyword-parameters-for-extension-functions

	const char *filename;
	char *mode = "r+";
	FILE *handle = NULL;
	void *fsi = NULL;

	imapi2fs_object *obj = NULL;

	static char *kwlist[] = {"filename", "mode", NULL};

	/*args should be filename and file open mode*/
	if (!PyArg_ParseTupleAndKeywords(args, keywds, "ss", kwlist, &filename, &mode))
	{
		return NULL;
	}

	//should have IMAPI2 FS object create this file... here as a placeholder for now
	handle = fopen(filename, mode);

	if (handle == NULL)
	{
		//create error if error opening file
	}
	
	fsi = fcreateIMAPI2FS();
	
	obj= newimapi2fs_object(args);

	obj->filename_str = PyString_FromString(filename);
	obj->IMAPI2FS_Object = fsi;
	obj->file_handle = handle;

//	Py_INCREF();
	
	return (PyObject *)obj;
}

/*
ex2_object
*/

//since c requires forward declare
//method table global
//python object methods and method table for module have similar structure and usage

static PyMethodDef IMAPI2FS_methods[] = {
	//python function name, actual/export function name, calling convention (METH_VARARGS, METH_NOARGS, or METH_VARARGS|METH_KEYWORDS or rarely old obsolete 0)
	{ "open", (PyCFunction)imapi2fs_open, METH_VARARGS | METH_KEYWORDS, "Create or open a new IMAPI2 FileSystem object with the provided filename and optionally with provided open mode" },
	//NULL termination block (SENTINEL)
	//{ NULL, NULL, 0, NULL }
	//another example has sentinel as
	{ NULL, NULL }
};

static PyObject *imapi2fs_new(PyObject *self, PyObject *args)
{
	imapi2fs_object *obj;

	if (!PyArg_ParseTuple(args, ":new"))
		return NULL;

	obj = newimapi2fs_object(args);

	if (obj == NULL)
		return NULL;

	return (PyObject *)obj;
}
static void imapi2fs_dealloc(imapi2fs_object *self)
{
	//based on source in xxmodule and _collections
	//in case ex2_object_attr is null, using xdecref
	Py_XDECREF(self->object_attr);
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

	Py_BEGIN_ALLOW_THREADS
	//	fprintf(fp, "ex2 object!", 0);
	Py_END_ALLOW_THREADS
	return status;
}

static PyObject *imapi2fs_getattr(imapi2fs_object *self, char *name)
{
	if (self->object_attr != NULL)
	{
		PyObject *attr = PyDict_GetItemString(self->object_attr, name);
		if (attr != NULL)
		{
			Py_INCREF(attr);
			return attr;
		}
	}

	//cast due to the call originally being of the modules object type
	return Py_FindMethod(IMAPI2FS_methods, (PyObject *)self, name);
}

static PyObject *imapi2fs_setattr(imapi2fs_object *self, char *name, PyObject *obj)
{

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



static PyTypeObject pyIMAPI2FS_type = {
	/* The ob_type field must be initialized in the module init function
	* to be portable to Windows without using C++. */
	//PyVarObject_HEAD_INIT(NULL, 0)
	PyObject_HEAD_INIT(NULL)
	0,//obj_size
	"pyIMAPI2FS.FileSystem",             /*tp_name*/
	sizeof(imapi2fs_object),          /*tp_basicsize*/
	0,                          /*tp_itemsize*/
	/* methods */
	(destructor)imapi2fs_dealloc, /*tp_dealloc*/
	(printfunc)imapi2fs_print,    /*tp_print*/
	(getattrfunc)imapi2fs_getattr, /*tp_getattr*/
	(setattrfunc)imapi2fs_setattr, /*tp_setattr*/
	0,                          /*tp_compare*/
	0,                          /*tp_repr*/
	0,                          /*tp_as_number*/
	0,                          /*tp_as_sequence*/
	0,                          /*tp_as_mapping*/
	0,                          /*tp_hash*/
	0,                      /*tp_call*/
	0,                      /*tp_str*/
	0,                      /*tp_getattro*/
	0,                      /*tp_setattro*/
	0,                      /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT,     /*tp_flags*/
	"pyIMAPI2 File System COM Object",/*tp_doc*/
	0,                      /*tp_traverse*/
	0,                      /*tp_clear*/
	0,                      /*tp_richcompare*/
	0,                      /*tp_weaklistoffset*/
	0,                      /*tp_iter*/
	0,                      /*tp_iternext*/
	0,                      /*tp_methods*/
	0,                      /*tp_members*/
	0,                      /*tp_getset*/
	0,                      /*tp_base*/
	0,                      /*tp_dict*/
	0,                      /*tp_descr_get*/
	0,                      /*tp_descr_set*/
	0,                      /*tp_dictoffset*/
	0,                      /*tp_init*/
	PyType_GenericAlloc,    /*tp_alloc*/
	/*newex2_object,                      /*tp_new*/
	PyType_GenericNew,
	PyObject_GC_Del,        /*tp_free*/
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
initpyIMAPI2FS(void)
{
	//declare module instance pointer
	PyObject *module_instance;

#ifdef __DEBUGBREAK__
	DebugBreak();
#endif

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

	Py_INCREF(&imapi2fs);
	
	PyModule_AddObject(module_instance, filesystem_type_name, (PyObject *)&pyIMAPI2FS_type);

	//PyObject_Print()
	//Py_DECREF();
	//ex1_object = PyObject_New()
}
