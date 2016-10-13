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

	hr = FileSystem->ChooseImageDefaultsForMediaType(IMAPI_MEDIA_TYPE_CDROM);

	if(hr != S_OK)
	{
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
}

char * CpyIMAPIObject::get_volume_name()
{
	return nullptr;
}

LONG CpyIMAPIObject::count()
{
	LONG count=0;
	
	current_directory->get_Count(&count);

	return count;
}

void CpyIMAPIObject::createISO()
{}

void CpyIMAPIObject::close()
{}

void CpyIMAPIObject::add(char *filename)
{
	wchar_t wfilename[257];
	size_t size;
	::mbstowcs_s(&size, wfilename, filename, 256);
	if (size > 255)
		return;

	IStream *file_stream = NULL;

	HRESULT hr = SHCreateStreamOnFileEx(wfilename,
								STGM_READ|STGM_SHARE_DENY_WRITE,
								FILE_ATTRIBUTE_NORMAL,
								FALSE, 
								NULL,
								&file_stream);

	IFsiFileItem *file_item = NULL;
	FileSystem->CreateFileItem(wfilename, &file_item);
	
	file_item->put_Data(file_stream);

	file_stream->Release();
	file_item->Release();
}

char **CpyIMAPIObject::list()
{
	HRESULT hr = S_OK;
	IEnumFsiItems *fsi_enum = NULL;
	current_directory->get_EnumFsiItems(&fsi_enum);


	char **paths;

	paths = new char *[count()];
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

		strcpy_s(paths[i], newsize, (char *)str);
		
	}

	return paths;

}

char *CpyIMAPIObject::getCWD()
{

	return "";
}

wchar_t * CpyIMAPIObject::getwCWD()
{
	return nullptr;
}

bool CpyIMAPIObject::exists(char * filename)
{
	return false;
}

void CpyIMAPIObject::freelist(char **list)
{
	int i = 0;
	while (list[i] != NULL)
		delete[] list[i];

	delete[] list;
}
void CpyIMAPIObject::remove(char *filename)
{

}