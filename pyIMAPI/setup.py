print("beginning setup.py, pip calls copytree and any open files not opened with shared read might interrupt this process, if this solution is open in visual studio, this will be the case")
import os
import sys
import platform

platform_string = " ".join(platform.uname()).lower()
module_name="pyIMAPI2FS"

print("platform uname: ", platform_string)

here = os.path.dirname(__file__)

print("the distribution has been copied for build and install to ", here)
#print("current working directory is ", os.getcwd())

ext_files = [#"pyIMAPI2FS_Module.c"
			]

kwargs = {}

from setuptools import setup, Extension

print("command line options to setup.py include ", sys.argv)

#    kwargs['cmdclass'] = {'build_ext': build_ext}

print("info: Building from C")

if "linux" in platform_string:
   
    print platform_string
    
    ext_modules = [Extension(module_name,
                        ext_files,
                        libraries=[])]

elif "windows" in platform_string:

    temp_directory = "c:\\temp"

#check for <drive>:\temp dir
    if not os.path.exists(temp_directory):
        #create temp dir if it does not exist
        os.makedirs(temp_directory)

#check for <drive>:\temp dir
#create temp dir if it does not exist
#lib openssl capi.lib and win32-mman
#print('mman-win32 headers in <current python>\\include, mman.lib required in <current python>\\libs')
#extension 1st ctor param is the name that must be used to import into python
    ext_modules =   [
                        Extension(module_name,
                        ext_files,#,
                        libraries=["pyIMAPI2FSutil"],
                        extra_compile_args=["/Zi"],#,"/LDd"],#if /LDd is specified, my setuptools and pip have the dll link occur to python27_d.lib
                        extra_link_args=["/DEBUG", "/PDB:c:\\temp\\"+module_name+".pdb"]),#libcmt warning for command line built lib linked here? already tried nodefaultlib....
                        #Extension("py_ex1",
                        #ext_files,#,
                        #libraries=[],
                        #extra_compile_args=[],
                        #extra_link_args=[])]#libcmt warning for command line built lib linked here? already tried nodefaultlib....
                    ]
ext_modules = []
requirements = []

if sys.version_info[0] < 3 and sys.version_info[1] < 7:
	requirements.append('importlib')

#name is the name of the package to pip and distutils
setup(name=module_name,
      #data_files=[('.', ['pyIMAPI2FSutil.dll'])],
      packages=["pyIMAPI2FS"],
      version="0.1",
      author="Kenneth Long",
      author_email="klong15@mail.greenriver.edu",
      #mercurial
      url="http://bitbucket.org/ken_long/pyIMAPI2FS/",
      description="A python module to provide a tarfile like object for creating ISO 9660 files using IMAPI2 FileSystem on Windows",
      license="MIT License",#"Creative Commons v3",
      dependency_links=["http://downloads.sourceforge.net/project/pywin32/pywin32/Build%20220/pywin32-220.zip?r=https%3A%2F%2Fsourceforge.net%2Fprojects%2Fpywin32%2Ffiles%2Fpywin32%2FBuild%2520220%2F&ts=1482970410&use_mirror=superb-sea2"],
      ext_modules=ext_modules,
      classifiers=[
          'Intended Audience :: Developers',
          'License :: OSI Approved :: MIT License',
          'Programming Language :: C',
          'Programming Language :: Python',
          'Topic :: Software Development :: Libraries :: Python Modules',
          'Development Status :: 2 - Pre-Alpha',
          'Environment :: Win32 (MS Windows)',
      ],
      #exclude_package_data = {'':['*.opendb']},
      **kwargs)