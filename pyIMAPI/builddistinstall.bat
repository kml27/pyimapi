
copy /y x64\Debug\pyIMAPI2FSutil.dll x64
copy /y x64\Debug\pyIMAPI2FSutil.lib x64

copy /y x86\Debug\pyIMAPI2FSutil.dll x86
copy /y x86\Debug\pyIMAPI2FSutil.lib x86

python setup.py sdist
pip install --upgrade ./dist/pyIMAPI2FS-0.1.tar.gz