// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <time.h>

//#include <windows.h>
//#import "file:imapi2.dll"
//#import "file:imapi2fs.dll"
#include <imapi2fs.h>
#include <imapi2fserror.h>

//#include <imapi2.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <VersionHelpers.h>

#include <Shlwapi.h>
#include <comutil.h>
//#include <conio.h>
#include <stdio.h>

// TODO: reference additional headers your program requires here
#include "pyIMAPI.h"