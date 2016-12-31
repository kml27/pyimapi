msbuild pyIMAPI.sln /m /t:Build /p:Configuration=Debug;Platform=x64
msbuild pyIMAPI.sln /m /t:Build /p:Configuration=Debug;Platform=x86

mkdir pyIMAPI2FS-dll
REM copy /y nul pyIMAPI2FS\__init__.py

mkdir pyIMAPI2FS-dll\x64
copy /y nul pyIMAPI2FS-dll\x64\__init__.py
copy /y x64\Debug\pyIMAPI2FSutil.dll pyIMAPI2FS-dll\x64
copy /y x64\Debug\pyIMAPI2FSutil.lib pyIMAPI2FS-dll\x64
copy /y x64\Debug\pyIMAPI2FSutil.pdb pyIMAPI2FS-dll\x64

mkdir pyIMAPI2FS-dll\x86
copy /y nul pyIMAPI2FS-dll\x86\__init__.py
copy /y win32\Debug\pyIMAPI2FSutil.dll pyIMAPI2FS-dll\x86
copy /y win32\Debug\pyIMAPI2FSutil.lib pyIMAPI2FS-dll\x86
copy /y win32\Debug\pyIMAPI2FSutil.pdb pyIMAPI2FS-dll\x86

python setup.py sdist
pip install --upgrade ./dist/pyIMAPI-0.1.tar.gz