// pyIMAPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <exception>
//#include "pyIMAPI.h"
// This is an example of an exported variable
//PYIMAPI_API int npyIMAPI=0;

char *HRtoStr(HRESULT hr)
{
	char *result = "Error not found";

	switch (hr)
	{
		case IMAPI_E_DIR_NOT_FOUND:
			result = "The dest directory was not found in FileSystemImage hierarchy.\n";
			break;
		case IMAPI_E_FILE_NOT_FOUND:
			result = "The file was not found in FileSystemImage hierarchy.\n";
			break;
		case IMAPI_E_IMAGE_SIZE_LIMIT:
			result = "Adding files would result in a result image having a size larger than the current configured limit.\n";
			break;
		case IMAPI_E_IMAGE_TOO_BIG:
			result = "Value specified for FreeMediaBlocks property is too small for estimated image size based on current data.\n";
			break;
		case IMAPI_E_ISO9660_LEVELS:
			result = "ISO9660 is limited to 8 levels of directories.\n";
			break;
		case IMAPI_E_RESTRICTED_NAME_VIOLATION:
			result = "The filename or path specified is not legal: Name of file or directory object created while the UseRestrictedCharacterSet property is set may only contain ANSI characters.\n";
			break;
		case IMAPI_E_BOOT_OBJECT_CONFLICT:
			result = "A boot object can only be included in an initial disc image.\n";
			break;
		case IMAPI_E_BOOT_IMAGE_DATA:
			result = "The boot object could not be added to the image.\n";
			break;
		case IMAPI_E_BOOT_EMULATION_IMAGE_SIZE_MISMATCH:
			result = "The emulation type requested does not match the boot image size.\n";
			break;
		case IMAPI_E_DUP_NAME:
			result = "The specified name already exists.\n";
			break;
		/*default:
			__assume(0);*/
	}

	return result;
}


CpyIMAPIObject::CpyIMAPIObject(char *filename, char *mode, char *disk_type, int bootable) : 
		output_file(NULL), 
		BootOptionsInstance(NULL)
{
	/*MsftRawCDImageCreator *cd_ic = NULL;
	LPVOID COMptr = NULL;
	CLSID RawCDImageCreatorCLSID;
	REFIID RCDIC_IID = __uuidof(MsftRawCDImageCreator);

	HRESULT hr = ERROR_SUCCESS;

	hr = CLSIDFromProgID(OLESTR("IMAPI2.MsftRawImageCreator"), &RawCDImageCreatorCLSID);

	hr = CoCreateInstance(RawCDImageCreatorCLSID, NULL, , (void **) &cd_ic);
	*/
	//IFileSystemImage *fsi = NULL;
	LPVOID COMptr = NULL;
	CLSID FSI_CLSID = __uuidof(MsftFileSystemImage);
	REFIID FSI_IID = __uuidof(IFileSystemImage);

	CLSID BootOpt_CLSID = __uuidof(BootOptions);
	REFIID BootOpt_IID = __uuidof(IBootOptions);

	HRESULT hr = S_OK;
#ifdef _DEBUG
	printf("DLL built at %s\n", __TIME__);
#endif
	//int len = GetCurrentDirectoryA(0, NULL);
	//char *cwd = new char[len+1];
	//GetCurrentDirectoryA(255, cwd);
	//printf("%s\n", cwd);
	//delete[] cwd;

	if (!::_strcmpi(mode, "r"))
	{
		//Read Only not supported by IMAPI
		return;
	}
	//hr = CLSIDFromProgID(OLESTR("IMAPI2FS.MsftFileSystemImage"), &FSI_CLSID);
	//printf("cocreateinstance");
	hr = CoCreateInstance(FSI_CLSID, NULL, CLSCTX_INPROC_SERVER, FSI_IID, (void **)&FileSystem);


	if (bootable)
	{
		//https://docs.microsoft.com/en-us/windows/desktop/imapi/adding-a-boot-image
		hr = CoCreateInstance(BootOpt_CLSID, NULL, CLSCTX_INPROC_SERVER, BootOpt_IID, (void **)&BootOptionsInstance);

		BootOptionsInstance->put_Manufacturer(_com_util::ConvertStringToBSTR(""));

		/*
		Constants
			PlatformX86	Intel Pentium™ series of chip sets. This entry implies a Windows operating system.
			PlatformPowerPC	Apple PowerPC family.
			PlatformMac	Apple Macintosh family.
			PlatformEFI	EFI Family.
		*/

		BootOptionsInstance->put_PlatformId(PlatformX86);

		/*
		Constants
			EmulationNone	No emulation. The BIOS will not emulate any device type or special sector size for the CD during boot from the CD.
			Emulation12MFloppy	Emulates a 1.2 MB floppy disk.
			Emulation144MFloppy	Emulates a 1.44 MB floppy disk.
			Emulation288MFloppy	Emulates a 2.88 MB floppy disk.
			EmulationHardDisk	Emulates a hard disk.
		*/

		BootOptionsInstance->put_Emulation(EmulationNone);

		//BootOptionsInstance->AssignBootImage()
		hr = FileSystem->put_BootImageOptions(BootOptionsInstance);

	}
	//todo: add support for other types of images like dvd and blu-ray
	//types specified in imapi2.h enum _IMAPI_MEDIA_PHYSICAL_TYPE
	//_IMAPI_MEDIA_PHYSICAL_TYPE
	
	/*IDiscFormat2::get_SupportedMediaTypes*/

	fs_type = FsiFileSystemISO9660;
	
	/*
	struct type_map{
		type_map(char *sz_in, int type_in) : sz(sz_in), type(type_in) {}
		char *sz;
		int type;
	} 
	*/
	char *sz_disk_types[] = {
		"CD",
		"DVD",
		"DVDDL",
		//HDDVD is failing to create, need to investigate
		//"HDDVD",
		"BluRay",
	};

	_IMAPI_MEDIA_PHYSICAL_TYPE imapi_disk_types[] = {
		IMAPI_MEDIA_TYPE_CDRW,
		IMAPI_MEDIA_TYPE_DVDPLUSRW,
		IMAPI_MEDIA_TYPE_DVDPLUSRW_DUALLAYER,
		//IMAPI_MEDIA_TYPE_HDDVDR,
		IMAPI_MEDIA_TYPE_BDR,
	};

	//default to most restrictive for now, can increase it in later releases and not break anything
	imapi_disk_type = IMAPI_MEDIA_TYPE_CDR;
	
	int j = sizeof(sz_disk_types)/sizeof(char *);

	for (int i = 0; i < j; i++)
	{
		if (::_strcmpi(disk_type, sz_disk_types[i]) == 0)
		{
			imapi_disk_type = imapi_disk_types[i];
		}
	}

	hr = FileSystem->put_FileSystemsToCreate(fs_type);
	
	hr = FileSystem->ChooseImageDefaultsForMediaType(imapi_disk_type);

	if(hr != S_OK)
	{
		if (hr == IMAPI_E_INVALID_PARAM)
		{
			printf("invalid parameter\n");
		}
		else if (hr == IMAPI_E_IMAGE_TOO_BIG)
		{
			printf("FreeMediaBlocks property is too small for estimated image size\n");
		}
		//release cocreateinstanced instance
		
		printf("error creating image\n");

		//assert(false);
		return;
	}

	//printf("get root");
	FileSystem->get_Root(&root);
	current_directory = root;

	//printf("%p\n", this);

	//fsi->put_FileSystemsToCreate(FsiFileSystemISO9660);
	//printf("exiting ctor\n");


	wchar_t wfilename[257];
	size_t size;
	::mbstowcs_s(&size, wfilename, filename, 256);
	
	DWORD grfMode = STGM_SHARE_DENY_WRITE;
	bool create = FALSE;

	if (!::_strcmpi(mode, "a"))
	{
		//'a' from python Tarfile docs
		/*
		Open for appending with no compression. The file is created if it does not exist.
		*/

//		PathFileExistsA

		//maybe these flags seem a little confusing, since we'd typically want to read/write for append
		//this implementation uses a tmp file, and we want to avoid overwriting existing files in the case
		//of a crash or failure (as to not lose info)
		//this grfMode combination will open the tmp file for write, but will not overwrite
		//it will also create the file if the file does not currently exist
		grfMode |= STGM_WRITE | STGM_CREATE; // STGM_FAILIFTHERE;
		create = true;
	}
	else if(!::_strcmpi(mode, "r"))
	{
		grfMode |= STGM_READ;

		//printf("Readonly not supported by IMAPI code. Bad code path.");
	}
	else if (!::_strcmpi(mode, "w"))
	{
		//always create and overwrite
		grfMode |= STGM_WRITE | STGM_CREATE;
		create = true;
		//FileSystem->ImportFileSystem
	}

	//STGOPTIONS
	//StgOpenStorageEx(wfilename, grfMode,)

	hr = SHCreateStreamOnFileEx(		wfilename, 
										grfMode, 
										FILE_ATTRIBUTE_NORMAL, 
										create, 
										NULL, 
										&output_file);
	

	//implementing powershell mount of iso for readonly

	//should continue investigating this option in light of Mount-DiskImage issue(s) (performance et c)

	//printf("create %s with mode %s had result %d", filename, mode, hr);
	//FileSystem->IdentifyFileSystemsOnDisc();
/*	IFileSystemImageResult *isofile = 
	IFileSystemImage  
	FileSystem->put_MultisessionInterfaces(image->get_MultisessionInterfaces())
	//FileSystem->ImportFileSystem();
	*/

	//printf("hr is %x\n", hr);

	return;
}

CpyIMAPIObject::~CpyIMAPIObject()
{
	if (root != NULL)
	{
		if (root != current_directory)
			current_directory->Release();

		root->Release();
	}

	if(output_file!=NULL)
		output_file->Release();
}

char* CpyIMAPIObject::set_volume_name(char * volumename)
{
	size_t size = strlen(volumename)+1;

	//The string is limited to 15 characters. 
	//If you do not specify a volume name, a default volume name is generated using the system date and time when the result object is created.

	//https://docs.microsoft.com/en-us/windows/desktop/api/imapi2fs/nf-imapi2fs-ifilesystemimage-put_volumename

	if (size > 15)
		return "Provided volume name exceeds maximum allowed 15 characters";

	//printf("volume name %s %d\n", volumename, size);

	BSTR bvolumename = _com_util::ConvertStringToBSTR(volumename);

	HRESULT hr = FileSystem->put_VolumeName(bvolumename);

	if (hr)
	{
		return HRtoStr(hr);
	}

	SysFreeString(bvolumename);

	return NULL;
}

char * CpyIMAPIObject::get_volume_name()
{
	//incomplete
	BSTR bstr_volumename;
	FileSystem->get_VolumeName(&bstr_volumename);

	int len = SysStringLen(bstr_volumename)+1;

	char *volumename = new char[len];
	//copy bstr to normal c str

	strcpy_s(volumename, len, _com_util::ConvertBSTRToString(bstr_volumename));

	//printf("getting name %s %d\n", volumename, len);

	return volumename;
}

char *CpyIMAPIObject::set_boot_sector(char *filename) 
{
	wchar_t wfilename[257];
	size_t size;

	::mbstowcs_s(&size, wfilename, filename, 256);

	IStream *file_stream = NULL;

	HRESULT hr = SHCreateStreamOnFileEx(wfilename,
		STGM_READ | STGM_SHARE_DENY_WRITE,
		FILE_ATTRIBUTE_NORMAL,
		FALSE,
		NULL,
		&file_stream);

	if (hr != S_OK)
	{
		//printf("hr is %x\n", hr);
		return HRtoStr(hr);
	}


	if (file_stream != NULL)
	{
		//wchar_t *wfullpath = getFullPath(wfilename);

		if (BootOptionsInstance)
		{
			hr = BootOptionsInstance->AssignBootImage(file_stream);
		}
		
		file_stream->Release();

		if (hr != S_OK)
		{
			//printf("hr is %x\n", hr);
			return HRtoStr(hr);
		}
	}

	return NULL;

}

LONG CpyIMAPIObject::count()
{
	LONG count=0;
	
	if (current_directory)
	{
		HRESULT hr = current_directory->get_Count(&count);
	}
	else
	{
#ifdef _DEBUG
		printf("error: current_directory object does not exists");
#endif
	}
	//FileSystem->get_FileCount(&count);

	//current_directory->get__NewEnum();
	//current_directory->get_EnumFsiItems();
	
	
	return count;
}

void CpyIMAPIObject::createISO()
{
	/*
	https://msdn.microsoft.com/en-us/library/windows/desktop/aa365646(v=vs.85).aspx

	Remarks

	Currently, IFileSystemImage::CreateResultImage will require disc media access as a result of a previous IFileSystemImage::IdentifyFileSystemsOnDisc method call. To resolve this issue, it is recommended that another IFileSystemImage object be created specifically for the IFileSystemImage::IdentifyFileSystemsOnDisc operation.
	The resulting stream can be saved as an ISO file if the file system is generated in a single session and has a start address of zero.
	*/

	if (!FileSystem)
		return;

	IFileSystemImageResult *iso;
	//instantiate imageresult
	HRESULT hr = FileSystem->CreateResultImage(&iso);

	IStream *isostream;
	iso->get_ImageStream(&isostream);
	
	LARGE_INTEGER offset;
	offset.LowPart = 0;
	offset.HighPart = 0;

	if (output_file) {
		output_file->Seek(offset, STREAM_SEEK_SET, NULL);

		STATSTG isostat;
		isostream->Stat(&isostat, FALSE);

		isostream->CopyTo(output_file, isostat.cbSize, NULL, NULL);
		isostream->Release();
	}
	else
	{
#ifdef _DEBUG
		printf("error: no output file was created, cannot write iso\n");
#endif
	}
}

void CpyIMAPIObject::close()
{
	createISO();

	if(output_file)
		output_file->Release();
}

void str_replace(char *path, char tok, char rpl) 
{
	for (char *c = path; c = strpbrk(c, &tok);) 
	{
		*c++ = rpl;
		
	}
}

bool absolute_path(char *path) 
{
	return '\\'== path[0] || ':' == path[1];
	//return strpbrk(path, "\\") == path;
}

int count_chr_below_alpha(char *path) 
{
	int count = 0;

	for (char *c = path; *c; c++)
	{
		if (*c < 65) 
		{
			count++;
		}
	}

	return count;
}

char *dup_backslash(char *raw_path)
{
	int count = count_chr_below_alpha(raw_path);

	size_t len = strlen(raw_path) + count + 1;

	char *dup_path = new char[len];

	for (char *c = raw_path, *cpy = dup_path; *c; c++, cpy++)
	{
		if (c[0] < 65)
		{
			cpy++[0] = '\\';
		}

		cpy[0] = c[0];
	}

	return dup_path;
}

void remove_dup_slash(char *path) 
{
	size_t len = strlen(path);

	char adv = 1;

	char *revised = path;

	for (char *orig = path; *orig; orig++, revised+=adv)
	{
		*revised = *orig;
		
		if (orig[0] == '\\' && orig[1] == '\\')
		{
			adv = 0;
		}
		else
		{
			adv = 1;
		}
	}
	//null terminate at new end of str
	revised[0] = '\0';
}

char *clean_path(char *base_path, char *path)
{

	//start with 2 for null and possible '\\'
	size_t max_len = 2;

	size_t base_len = strlen(base_path);
	size_t path_len = strlen(path);

	printf("base_path %s len %zd path %s len %zd\n", base_path, base_len, path, path_len);

	max_len += base_len + path_len;

	char *complete_path = new char [max_len];

	//str_replace(base_path, '/', '\\');
	str_replace(path, '/', '\\');
	
	printf("after slash replace %s\n", path);

	if (!absolute_path(path)) 
	{
		printf("not abs path\n");

		char *concat_path = "%s\\%s";

		if (base_path[base_len-1] == '\\')
		{
			concat_path = "%s%s";

			printf("base path ends in \\\n");
		}

		printf("not abs path, before sprintf, %zd\n", max_len);

		sprintf_s(complete_path, max_len, concat_path, base_path, path);

		printf("not abs path, completepath %s\n", complete_path);
	}
	else 
	{
		printf("abs path\n");
		
		sprintf_s(complete_path, max_len, "%s", path);
		
		printf("abs path, completepath %s\n", complete_path);
	}

	remove_dup_slash(complete_path);

	printf("after remove dup completepath %s\n", complete_path);

	max_len = strlen(complete_path);

	//if this isn't the root of a letter drive, strip trailing slash
	if (complete_path[max_len - 2] != ':' && complete_path[max_len - 1] == '\\' )
	{
		//null terminate early, IMAPI doesnt like trailing slashes for directories
		complete_path[max_len - 1] = '\0';
	}

	printf("final completepath %s\n", complete_path);

	return complete_path;
}

/*
char* CpyIMAPIObject::add(char *filename, char *dest)
{}
*/

char* CpyIMAPIObject::add(char *filename, int add_dir)
{
	char *result = "File could not be added. File not found";

	wchar_t wfilename[257];
	size_t size = strlen(filename);

	if (size > 255)
		return "Filename exceeds 255 characters.\n";


	//printf("C++ iso add, %p\n", this);

	char path[256];

	::_getcwd(path, 256);

	char *fullpath = clean_path(path, filename);

	wchar_t wfullpath[512];
	
	::mbstowcs_s(&size, wfullpath, fullpath, 256);
	
	delete[]fullpath;

	wprintf(L"%s\n", wfullpath);

	BSTR bfilename = NULL;

	if (PathIsDirectory(wfilename))
	{
		//add tree
		printf("add tree, %s, %p, %d\n", filename, current_directory, add_dir);

		//source directory is specified to AddTree
		bfilename = _com_util::ConvertStringToBSTR(filename);
		
		HRESULT hr = S_OK;
		
		hr = current_directory->AddTree(bfilename, add_dir);// TRUE);
		
		printf("added tree\n");

		if (hr != S_OK)
		{
			printf("hr is %x\n", hr);
			return HRtoStr(hr);
		}

	}
	else
	{
		if (!PathFileExists(wfilename))
		{
			return "File does not exist.\n";
		}

		char *c = filename, *filename_to_use = filename;

		if (c = strchr(filename, ':'))
		{
			filename_to_use = c + 1;
		}

		//printf("path after drive is %s\n", filename_to_use);

		//path
		bfilename = _com_util::ConvertStringToBSTR(filename_to_use);

		IStream *file_stream = NULL;

		//printf("adding single file to iso\n");

		HRESULT hr = SHCreateStreamOnFileEx(wfilename,
			STGM_READ | STGM_SHARE_DENY_WRITE,
			FILE_ATTRIBUTE_NORMAL,
			FALSE,
			NULL,
			&file_stream);

		/*
		HRESULT AddFile(
		[in] BSTR    path,
		[in] IStream *fileData
		);

		Parameters

		path [in]
		String that contains the relative path of the directory to contain the new file.
		Specify the full path when calling this method from the root directory item.
		fileData [in]
		An IStream interface of the file (data stream) to write to the media.
		*/
		if (hr != S_OK) 
		{
			//printf("hr is %x\n", hr);
			return HRtoStr(hr);
		}

		
		if (file_stream != NULL)
		{
			//printf("filestream is not null\n");

			//wchar_t *wfullpath = getFullPath(wfilename);

			//printf("current directory object is %p\n", current_directory);

			//IFsiFileItem *file_item = NULL;
			if (current_directory) 
			{
				hr = current_directory->AddFile(bfilename, file_stream);
			}

			
			//IFsiFileItem *out;
		//	FileSystem->CreateFileItem(wfilename, &out);

			//out->put_Data(file_stream);

			file_stream->Release();

			if (hr != S_OK)
			{
				//printf("hr is %x\n", hr);
				return HRtoStr(hr);
			}

			//out->Release();
			//result = NULL;


		}


	}
	
	printf("after add branches\n");

	SysFreeString(bfilename);

	return NULL;
}

char **CpyIMAPIObject::list()
{
	HRESULT hr = S_OK;
	IEnumFsiItems *fsi_enum = NULL;
	current_directory->get_EnumFsiItems(&fsi_enum);


	char **paths;

	long num_items = count();

	if (num_items < 1)
		return NULL;

	paths = new char *[num_items+1];
	::memset(paths, 0, sizeof(char *)*(num_items+1));
	int i = 0;
	IFsiItem *item;
	ULONG return_count = 0;

	for (i=0;S_OK == fsi_enum->Next(1, &item, &return_count) && return_count==1;i++)
	{
		BSTR str = NULL;

		item->get_FullPath(&str);
		//https://msdn.microsoft.com/en-us/library/ms235631.aspx
		
		//allocated with new []
		paths[i] = _com_util::ConvertBSTRToString(str);
		//printf("%s", paths[i]);
	}

	return paths;

}

char *CpyIMAPIObject::getCWD()
{
	//printf("%p", this);
	//printf("in getcwd dll\n");
	BSTR bpath = NULL;
	//current internal image file path
	current_directory->FileSystemPath(fs_type, &bpath);
	char *path = NULL;

	//directory stash files are built in
	//FileSystem->get_WorkingDirectory(&bpath);
	int len = SysStringLen(bpath);//*2;
	
	//if more than null terminated str
	if (len > 0)
	{
		//printf("%d\n", len);
		//allocated with new []
		char *original = _com_util::ConvertBSTRToString(bpath);
		path = new char[len + 3];
		strcpy_s(path, len + 3, original);
		delete[] original;
	}
	else if (current_directory == root)
	{
		//these strs will always be freed with delete[], provide a new[] str for that purpose
		//printf("root");
		path = new char[3];
	}
	else
	{
		//printf("no dir\n");
	}
	
	strcpy_s(&path[len], 3, "\\");

	return path;
}

wchar_t * CpyIMAPIObject::getwCWD()
{
	printf("unimplemented wide char cwd\n");
	return nullptr;
}

char *CpyIMAPIObject::setCWD(char * path)
{
	bool absolute = false;
	bool free_path = false;

	//if the path starts with root, use absolute path
	//if (strpbrk(path, "\\")==path)
	//	absolute = true;
	absolute = absolute_path(path);

	//if the strings are equal the diff is 0, not to test for equiv 
	//should use strtok/strpbrk
	if (!strcmp(path, ".."))
	{
		char *cwd = getCWD();
		char *context, *cur, *prev;
		
		cur = strtok_s(cwd, "\\", &context);
		
		while (cur)
		{
			printf("%s\n", cur);
			prev = cur;

			cur = strtok_s(NULL, "\\", &context);
			
			if (cur == NULL)
				*prev = NULL;
		}

		path = cwd;

		//printf("setting from absolute to %s\n", path);
		free_path = true;
	}

	//printf("in setcwd dll\n");
	//path
	BSTR bpath = _com_util::ConvertStringToBSTR(path);
	IFsiDirectoryItem *dir = NULL;
	IFsiItem *item = NULL;

	//should be absolute/relative
	if (absolute)
	{
		//printf("setting from root to %s\n", path);
		root->get_Item(bpath, &item);
		//printf("set from root\n");
	}
	else
	{
		current_directory->get_Item(bpath, &item);
	}

	SysFreeString(bpath);

	if (item != NULL)
	{
		item->QueryInterface(__uuidof(IFsiDirectoryItem), (void**)&dir);
	}

	char *result = "no such directory";

	if (dir != NULL)
	{
		
		if (current_directory != root)
		{
			//printf("release directory\n");
			current_directory->Release();
		}
		current_directory = dir;
		result = S_OK;
	}
	else
	{
		//throw python exception!
	}

	if (free_path)
		delete[]path;

	return result;
}

char *CpyIMAPIObject::mkdir(char *path)
{
	//printf("%p\n", this);

	//printf("in dll mkdir\n");

	if (path == NULL || strlen(path) == 0)
	{
		return "null or empty str";
	}

	if (FileSystem == nullptr || current_directory == nullptr)
	{
		//printf("nullptr member\n");
		return "FileSystem not initialized properly\n";
	}
	
	//printf("%s\n", path);
	
	BSTR bpath = _com_util::ConvertStringToBSTR(path);
	IFsiDirectoryItem *dir;
	
	//printf("converted string\n");
	//printf("%p\n", FileSystem);
	HRESULT hr = FileSystem->CreateDirectoryItem(bpath, &dir);
	//printf("created directory\n");
	current_directory->Add(dir);
	//printf("added directory\n");
	//current_directory->AddDirectory(bpath);

	dir->Release();
	//printf("released directory item\n");
	SysFreeString(bpath);

	/*
	Return code	Description
E_POINTER
Pointer is not valid.

Value: 0x80004003

IMAPI_E_INVALID_PARAM
The value specified for parameter %1!ls! is not valid.

Value: 0xC0AAB101

E_OUTOFMEMORY
Failed to allocate the required memory.

Value: 0x8007000E
	*/

	char *result_path = path;

	switch (hr) {
		case E_POINTER:
			result_path = "";
			break;

		case IMAPI_E_INVALID_PARAM:
			result_path = "";
			break;
	
		case E_OUTOFMEMORY:
			result_path = "";
			break;

		case S_OK:
			break;
		default:
			result_path = "";
			break;

	}
	

	//printf("freed bstr\n");
	return path;
}

bool CpyIMAPIObject::exists(char *filename)
{
	//char fullpath[512];

	//printf("in exists dll\n");
	
	char *cwd = getCWD();
	
	//printf("filename %s, after getcwd, cwd %s\n", filename, cwd);

	//windows already behaves as is without dup_backslash, 
	//and no way besides direct map to convert back from special escaped chars?
	//char *filename = dup_backslash(raw_filename);

	//printf("dup_backslash filename %s\n", filename);

	char *fullpath = clean_path(cwd, filename);
	
	//printf("after sprintf, %s\n", fullpath);
	
	delete []cwd;
	//path
	BSTR bfilename = _com_util::ConvertStringToBSTR(filename);
	BSTR bfullpath = _com_util::ConvertStringToBSTR(fullpath);

	FsiItemType type;

	//following seems to work for file
	FileSystem->Exists(bfullpath, &type);

	//IFsiItem *file;
	//not sure why this was here?
	//current_directory->get_Item(bfilename, &file);

	bool item_exists = false;//file != NULL;

	switch (type)
	{
		case FsiItemNotFound:
			
			break;

		case FsiItemDirectory:
		case FsiItemFile:
			item_exists = true;
			break;
	}

	
/*	if (item_exists)
		file->Release();
		*/

	SysFreeString(bfilename);
	SysFreeString(bfullpath);

	delete[]fullpath;

	return item_exists;
}

void CpyIMAPIObject::freelist(char **list)
{
	int i = 0;

	if (list == NULL)
		return;

	while (list[i] != NULL)
		delete[] list[i++];

	delete[] list;
}

int CpyIMAPIObject::remove(char *filename)
{

	char *cwd = getCWD();

	char *fullpath = clean_path(cwd, filename);

	//BSTR bfilename = _com_util::ConvertStringToBSTR(filename);

	BSTR bfilename = _com_util::ConvertStringToBSTR(fullpath);

	delete[]fullpath;

	//is a check for directory and RemoveTree required? IMAPI_E_DIR_NOT_EMPTY

	//IMAPI_E_ITEM_NOT_FOUND

	HRESULT hr = current_directory->Remove(bfilename);

	SysFreeString(bfilename);


	if (hr == S_OK)
		return 1;
	
	return 0;
}

//should create struct or ** of dict entries
void *CpyIMAPIObject::next()
{
	return NULL;
}
/*
int CpyIMAPIObject::extract(char *filename, char *dest_system_path)
{
	int result = FALSE;
	wchar_t wpath[512];

	wchar_t wfilename[256];
	size_t size;
	::mbstowcs_s(&size, wfilename, filename, 256);

	if (dest_system_path == NULL)
	{
		GetCurrentDirectoryW(512, wpath);
	}
	else
	{
		//printf("using provided path, %s\n", dest_system_path);
		size_t size;
		::mbstowcs_s(&size, wpath, dest_system_path, 512);
	}

	wsprintf(wpath, L"%s\\%s", wpath, wfilename);
	//wprintf(L"%s\n", wpath);

	DWORD grfMode = STGM_SHARE_DENY_WRITE;
	bool create = TRUE;

	grfMode |= STGM_WRITE | STGM_FAILIFTHERE;

	HRESULT hr = SHCreateStreamOnFileEx(wpath,
		grfMode,
		FILE_ATTRIBUTE_NORMAL,
		create,
		NULL,
		&output_file);

	//printf("opened output file with result %d\n", hr);
	if (output_file != NULL)
	{
		//printf("getting iso item\n");
		BSTR bpath = _com_util::ConvertStringToBSTR(filename);
		IFsiItem *item = NULL;

		hr = current_directory->get_Item(bpath, &item);
		
		if (item != NULL)
		{
			printf("getting iso file item\n");

			//IFsiDirectoryItem *dir = NULL;
			//item->QueryInterface(__uuidof(IFsiDirectoryItem), (void**)&dir);

			IFsiFileItem *file = NULL;
			item->QueryInterface(__uuidof(IFsiFileItem), (void**)&file);

			if (file != NULL)
			{

				printf("getting data stream\n");

				IStream *data = NULL;
				file->get_Data(&data);
				
				if (data != NULL)
				{
					printf("copying data stream to system\n");
					STATSTG data_stat;

					data->Stat(&data_stat, FALSE);
					printf("filesize %I64u\n", *(unsigned __int64 *) &data_stat.cbSize);
					HRESULT hr = data->CopyTo(output_file, data_stat.cbSize, NULL, NULL);

					printf("copyto result %d\n", hr);

					data->Release();
					file->Release();
					
					result = TRUE;
				}

			}

			item->Release();
		}

		output_file->Release();
		SysFreeString(bpath);
	}
	
	return result;
}

*/