@echo OFF
echo PYTHONPATH Environment variable must be defined.
IF DEFINED PYTHONPATH (echo Environment variable pythonpath is set to %PYTHONPATH%) ELSE exit /b 255
echo ON

msbuild pyIMAPI.sln /m /t:Build /p:Configuration=Release;Platform=x64
msbuild pyIMAPI.sln /m /t:Build /p:Configuration=Release;Platform=x86

mkdir pyIMAPI2FS-dll
REM copy /y nul pyIMAPI2FS\__init__.py

mkdir pyIMAPI2FS-dll\x64
REM copy /y nul pyIMAPI2FS-dll\x64\__init__.py
copy /y x64\Release\pyIMAPI2FSutil.dll pyIMAPI2FS-dll\x64
copy /y x64\Release\pyIMAPI2FSutil.lib pyIMAPI2FS-dll\x64
copy /y x64\Release\pyIMAPI2FSutil.pdb pyIMAPI2FS-dll\x64

mkdir pyIMAPI2FS-dll\x86
REM copy /y nul pyIMAPI2FS-dll\x86\__init__.py
copy /y win32\Release\pyIMAPI2FSutil.dll pyIMAPI2FS-dll\x86
copy /y win32\Release\pyIMAPI2FSutil.lib pyIMAPI2FS-dll\x86
copy /y win32\Release\pyIMAPI2FSutil.pdb pyIMAPI2FS-dll\x86

python setup.py sdist
pip install --upgrade ./dist/pyIMAPI-0.1.tar.gz