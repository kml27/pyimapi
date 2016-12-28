// pyIMAPI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
//#include "pyIMAPI.h"
// This is an example of an exported variable
//PYIMAPI_API int npyIMAPI=0;

// This is an example of an exported function.

CpyIMAPIObject::CpyIMAPIObject()
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

	//hr = CLSIDFromProgID(OLESTR("IMAPI2FS.MsftFileSystemImage"), &FSI_CLSID);

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
			printf("invalid parameter");
		else if (hr == IMAPI_E_IMAGE_TOO_BIG)
			printf("FreeMediaBlocks property is too small for estimated image size");

		//release cocreateinstanced instance
		
		//assert(false);
		return;
	}

	IFsiDirectoryItem *root = NULL;

	FileSystem->get_Root(&root);
	current_directory = root;

	//fsi->put_FileSystemsToCreate(FsiFileSystemISO9660);

	return;
}

void CpyIMAPIObject::set_volume_name(char * volumename)
{
	size_t size = strlen(volumename);
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

	int len = SysStringLen(bstr_volumename);

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
{}

void CpyIMAPIObject::close()
{}

/*
char* CpyIMAPIObject::add(char *filename, char *dest)
{}
*/

char* CpyIMAPIObject::add(char *filename)
{
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
	BSTR bfilename = SysAllocString(wfilename);

	int len = GetCurrentDirectoryA(0, NULL);
	char *cwd = new char[len];
	GetCurrentDirectoryA(255, cwd);

	IStream *file_stream = NULL;

	HRESULT hr = SHCreateStreamOnFileEx(wfilename,
								STGM_READ|STGM_SHARE_DENY_WRITE,
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
	char *result = "File could not be added. File not found";

	if (file_stream != NULL)
	{
		//wchar_t *wfullpath = getFullPath(wfilename);

		//IFsiFileItem *file_item = NULL;
		hr = current_directory->AddFile(bfilename, file_stream);
		IFsiFileItem *out;
	//	FileSystem->CreateFileItem(wfilename, &out);

		//out->put_Data(file_stream);

		file_stream->Release();
		//out->Release();
		result = "File Added";
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

	while (S_OK == fsi_enum->Next(1, &item, &return_count) && return_count==1)
	{
		BSTR str = NULL;

		item->get_FullPath(&str);
		//https://msdn.microsoft.com/en-us/library/ms235631.aspx
		size_t newsize = (SysStringLen(str) + 1) * 2;
		paths[i] = new char[newsize];

		strcpy_s(paths[i], newsize, _com_util::ConvertBSTRToString(str));
		
	}

	return paths;

}

char *CpyIMAPIObject::getCWD()
{
	BSTR bpath;
	//current internal image file path
	current_directory->FileSystemPath(fs_type, &bpath);
	//directory stash files are built in
	//FileSystem->get_WorkingDirectory(&bpath);
	int len = (SysStringLen(bpath)+1)*2;
	char *path = new char[len];

	strcpy_s(path, len, _com_util::ConvertBSTRToString(bpath));

	return path;
}

wchar_t * CpyIMAPIObject::getwCWD()
{
	return nullptr;
}

char *CpyIMAPIObject::setCWD(char * path)
{
	wchar_t wpath[257];
	size_t size;
	::mbstowcs_s(&size, wpath, path, 256);

	//path
	BSTR bpath = SysAllocString(wpath);
	IFsiDirectoryItem *dir;
	IFsiItem *item;
	
	current_directory->get_Item(bpath, &item);

	SysFreeString(bpath);

	item->QueryInterface(__uuidof(IFsiDirectoryItem), (void**) &dir);
	
	char *result = "no such directory";

	if (dir != NULL)
	{
		current_directory->Release();
		current_directory = dir;
		result = "changed to directory";
	}

	return result;
}

char *CpyIMAPIObject::mkdir(char *path)
{
	wchar_t wpath[257];
	size_t size;
	::mbstowcs_s(&size, wpath, path, 256);

	//path
	BSTR bpath = SysAllocString(wpath);
	IFsiDirectoryItem *dir;
	FileSystem->CreateDirectoryItem(bpath, &dir);
	
	current_directory->Add(dir);
	//current_directory->AddDirectory(bpath);

	dir->Release();

	SysFreeString(bpath);

	return "nochk";
}

bool CpyIMAPIObject::exists(char * filename)
{
	IFsiItem *file;
	
	wchar_t wfilename[257];
	size_t size;
	::mbstowcs_s(&size, wfilename, filename, 256);

	wchar_t wfullpath[512];

	::mbstowcs_s(&size, wfullpath, getCWD(), 512);

	wsprintf(wfullpath, L"%s\\%s", wfullpath, wfilename);

	//path
	BSTR bfilename = SysAllocString(wfilename);
	BSTR bfullpath = SysAllocString(wfullpath);
	
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
void CpyIMAPIObject::remove(char *filename)
{
	BSTR bfilename = _com_util::ConvertStringToBSTR(filename);

	current_directory->Remove(bfilename);

	SysFreeString(bfilename);
}