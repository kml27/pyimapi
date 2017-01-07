// pyIMAPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
//#include "pyIMAPI.h"
// This is an example of an exported variable
//PYIMAPI_API int npyIMAPI=0;

// This is an example of an exported function.

CpyIMAPIObject::CpyIMAPIObject(char *filename, char *mode) : output_file(NULL)
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

	HRESULT hr = S_OK;
#ifdef _DEBUG
	printf("DLL built at %s\n", __TIME__);
#endif
	//int len = GetCurrentDirectoryA(0, NULL);
	//char *cwd = new char[len+1];
	//GetCurrentDirectoryA(255, cwd);
	//printf("%s\n", cwd);
	//delete[] cwd;

	//hr = CLSIDFromProgID(OLESTR("IMAPI2FS.MsftFileSystemImage"), &FSI_CLSID);
	//printf("cocreateinstance");
	hr = CoCreateInstance(FSI_CLSID, NULL, CLSCTX_INPROC_SERVER, FSI_IID, (void **)&FileSystem);

	//todo: add support for other types of images like dvd and blu-ray
	//types specified in imapi2.h enum _IMAPI_MEDIA_PHYSICAL_TYPE
	//_IMAPI_MEDIA_PHYSICAL_TYPE
	
	fs_type = FsiFileSystemISO9660;

	hr = FileSystem->put_FileSystemsToCreate(fs_type);
	
	hr = FileSystem->ChooseImageDefaultsForMediaType(IMAPI_MEDIA_TYPE_CDR);

	if(hr != S_OK)
	{
		if (hr == IMAPI_E_INVALID_PARAM)
			printf("invalid parameter\n");
		else if (hr == IMAPI_E_IMAGE_TOO_BIG)
			printf("FreeMediaBlocks property is too small for estimated image size\n");

		//release cocreateinstanced instance
		
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

	if (!strcmp(mode, "a"))
	{
		//'a' from python Tarfile docs
		/*
		Open for appending with no compression. The file is created if it does not exist.
		*/
		grfMode |= STGM_READWRITE | STGM_CREATE;
		create = TRUE;
	}
	else if(!strcmp(mode, "r"))
	{
		printf("readonly file\n");
		grfMode |= STGM_READ;

		printf("Readonly not supported by IMAPI code. Bad code path.");
	}
	else if (!strcmp(mode, "w"))
	{
		printf("writeonly file");
		grfMode |= STGM_WRITE;
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

	//printf("create %s with mode %s had result %d", filename, mode, hr);
	//FileSystem->IdentifyFileSystemsOnDisc();
/*	IFileSystemImageResult *isofile = 
	IFileSystemImage  
	FileSystem->put_MultisessionInterfaces(image->get_MultisessionInterfaces())
	//FileSystem->ImportFileSystem();
	*/

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
void CpyIMAPIObject::set_volume_name(char * volumename)
{
	size_t size = strlen(volumename)+1;
	if (size > 255)
		return;

	BSTR bvolumename = _com_util::ConvertStringToBSTR(volumename);

	HRESULT hr = FileSystem->put_VolumeName(bvolumename);

	SysFreeString(bvolumename);
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

	return volumename;
}

LONG CpyIMAPIObject::count()
{
	LONG count=0;
	
	HRESULT hr = current_directory->get_Count(&count);

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

	IFileSystemImageResult *iso;
	//instantiate imageresult
	HRESULT hr = FileSystem->CreateResultImage(&iso);

	IStream *isostream;
	iso->get_ImageStream(&isostream);
	
	LARGE_INTEGER offset;
	offset.LowPart = 0;
	offset.HighPart = 0;

	output_file->Seek(offset, STREAM_SEEK_SET, NULL);
	
	STATSTG isostat;
	isostream->Stat(&isostat, FALSE);

	isostream->CopyTo(output_file, isostat.cbSize, NULL, NULL);
	isostream->Release();
	
}

void CpyIMAPIObject::close()
{
	createISO();

	if(output_file)
		output_file->Release();
}

/*
char* CpyIMAPIObject::add(char *filename, char *dest)
{}
*/

char* CpyIMAPIObject::add(char *filename)
{
	char *result = "File could not be added. File not found";

	wchar_t wfilename[257];
	size_t size;
	::mbstowcs_s(&size, wfilename, filename, 256);

	if (size > 255)
		return "Filename exceeds 255 characters";
	/*
	wchar_t wfullpath[512];

	::mbstowcs_s(&size, wfullpath, getCWD(), 512);

	wsprintf(wfullpath, L"%s\\%s", wfullpath, wfilename);
	*/
	//path
	BSTR bfilename = _com_util::ConvertStringToBSTR(filename);

	IStream *file_stream = NULL;

	if (PathIsDirectory(wfilename))
	{
		//add tree
		//printf("add tree\n");
		current_directory->AddTree(bfilename, TRUE);
		result = NULL;
	}
	else
	{
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
		
		if (file_stream != NULL)
		{
			//wchar_t *wfullpath = getFullPath(wfilename);

			//IFsiFileItem *file_item = NULL;
			hr = current_directory->AddFile(bfilename, file_stream);
			//IFsiFileItem *out;
		//	FileSystem->CreateFileItem(wfilename, &out);

			//out->put_Data(file_stream);

			file_stream->Release();
			//out->Release();
			result = NULL;
		}


	}
	
	SysFreeString(bfilename);

	return result;
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
	int len = (SysStringLen(bpath) + 1);//*2;
	
	//if more than null terminated str
	if (len > 1)
	{
		//printf("%d\n", len);
		//allocated with new []
		path = _com_util::ConvertBSTRToString(bpath);
	}
	else if (current_directory == root)
	{
		//these strs will always be freed with delete[], provide a new[] str for that purpose
		//printf("root");
		path = new char[3];
		strcpy_s(path, 3, "\\");
	}
	else
	{
		printf("no dir");
	}

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
	if (strpbrk(path, "\\")==path)
		absolute = true;

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
		printf("nullptr member\n");
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
	//printf("freed bstr\n");
	return "nochk";
}

bool CpyIMAPIObject::exists(char * filename)
{
	IFsiItem *file;
	
	char fullpath[512];

	//printf("in exists dll\n");
	char *cwd = getCWD();
	//printf("after getcwd\n");

	sprintf_s(fullpath, 512, "%s\\%s", cwd, filename);
	//printf("after sprintf\n");
	delete []cwd;
	//path
	BSTR bfilename = _com_util::ConvertStringToBSTR(filename);
	BSTR bfullpath = _com_util::ConvertStringToBSTR(fullpath);
	
	FsiItemType type;

	FileSystem->Exists(bfullpath, &type);

	current_directory->get_Item(bfilename, &file);

	switch (type)
	{
		case FsiItemNotFound:
			break;
		case FsiItemDirectory:
			break;
		case FsiItemFile:
			break;
	}

	bool item_exists = file != NULL;
	
	if (item_exists)
		file->Release();

	SysFreeString(bfilename);
	SysFreeString(bfullpath);

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
	BSTR bfilename = _com_util::ConvertStringToBSTR(filename);

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
		printf("using provided path\n");
		size_t size;
		::mbstowcs_s(&size, wpath, dest_system_path, 512);
	}

	wsprintf(wpath, L"%s\\%s", wpath, wfilename);
	wprintf(L"%s\n", wpath);

	DWORD grfMode = STGM_SHARE_DENY_WRITE;
	bool create = FALSE;

	char *mode = "a";

	if (!strcmp(mode, "a"))
	{
		//'a' from python Tarfile docs
		/*
		Open for appending with no compression. The file is created if it does not exist.
		*/
		grfMode |= STGM_READWRITE | STGM_CREATE;
		create = TRUE;
	}
	else if (!strcmp(mode, "r"))
	{
		grfMode |= STGM_READ;
	}
	else if (!strcmp(mode, "w"))
	{
		grfMode |= STGM_WRITE;
	}

	HRESULT hr = SHCreateStreamOnFileEx(wpath,
		grfMode,
		FILE_ATTRIBUTE_NORMAL,
		create,
		NULL,
		&output_file);

	printf("opened output file with result %d\n", hr);
	if (output_file != NULL)
	{
		printf("getting iso item\n");
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
					printf("filesize %d\n", data_stat.cbSize);
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