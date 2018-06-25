from __future__ import print_function

print("beginning setup.py, pip calls copytree and any open files not opened with shared read might interrupt this process, if this solution is open in visual studio, this will be the case")
import os
import sys
import platform

platform_string = " ".join(platform.uname()).lower()
module_name="pyIMAPI"

print("platform uname: ", platform_string)

here = os.path.dirname(__file__)

print("the distribution has been copied for build and install to ", here)
#print("current working directory is ", os.getcwd())

ext_files = ["pyIMAPI2FS_Module.c",
            ]

kwargs = {}

from setuptools import setup, Extension

print("command line options to setup.py include ", sys.argv)

#    kwargs['cmdclass'] = {'build_ext': build_ext}

print("info: Building from C")
arch = ""
if "linux" in platform_string:
   
    print(platform_string)
    
    ext_modules = [Extension(module_name,
                        ext_files,
                        libraries=[])]

elif "windows" in platform_string:
    if "amd64" in platform_string:
        arch="x64"
    else:
        arch="x86"
    temp_directory = "c:\\temp"

    #check for <drive>:\temp dir
    if not os.path.exists(temp_directory):
        #create temp dir if it does not exist
        os.makedirs(temp_directory)

    #extension 1st ctor param is the name that must be used to import into python
    ext_modules =   [
                        Extension(module_name,
                        ext_files,#,
                        libraries=["pyIMAPI2FS-dll\\"+arch+"\\pyIMAPI2FSutil","kernel32", "ntdll"],
                        extra_compile_args=["/Zi"],#,"/LDd"],#if /LDd is specified, my setuptools and pip have the dll link occur to python27_d.lib
                        extra_link_args=["/DEBUG", "/PDB:c:\\temp\\"+module_name+".pdb"]),#libcmt warning for command line built lib linked here? already tried nodefaultlib....
                    ]
requirements = []

if sys.version_info[0] < 3 and sys.version_info[1] < 7:
	requirements.append('importlib')

with open("README.md", "r") as fh:
    long_description = fh.read()

#evaluated during wheel build, this doesnt work for distro
#import site
#site_path = [path for path in site.getsitepackages() if "site-packages" in path][0]

#name is the name of the package to pip and distutils
setup(name=module_name,
      #adding site_path fixes local pip install ./pyIMAPI data dir but breaks distro with wheel/twine
      #just build with python setup.py sdist bdist_wheel and pip install dist\<wheel package or sdist> to test
      data_files=[("\\", ["pyIMAPI2FS-dll\\"+arch+"\\pyIMAPI2FSutil.dll"])],
      version="0.4.0b2",
      author="Kenneth Long",
      author_email="kennethlong@acm.org",
      url="http://bitbucket.org/ken_long/pyIMAPI2FS/",
      description="Create CD, DVD, DVD Dual Layer, and BluRay ISO images under Windows directly from Python",
      long_description = long_description,
      license="MIT License",
      #test_suite='tests.test_all',
      install_requires=requirements,
      ext_modules=ext_modules,
      classifiers=[
          'Intended Audience :: Developers',
          'Intended Audience :: Information Technology',
          'Intended Audience :: System Administrators',
          'License :: OSI Approved :: MIT License',
          'Programming Language :: C',
          'Programming Language :: C++',
          'Programming Language :: Python',
          'Programming Language :: Python :: 2.7',
          'Programming Language :: Python :: 3.4',
          'Programming Language :: Python :: 3.5',
          'Programming Language :: Python :: 3.6',
          'Programming Language :: Python :: 3.7',
          'Topic :: Multimedia',
          'Topic :: Software Development :: Libraries',
          'Topic :: Software Development :: Libraries :: Python Modules',
          'Topic :: Software Development :: Build Tools',
          'Topic :: System :: Archiving',
          'Topic :: System :: Archiving :: Backup',
          'Topic :: System :: Filesystems',
          'Topic :: System :: Software Distribution',
          'Topic :: Utilities',
          'Development Status :: 4 - Beta',
          'Operating System :: Microsoft :: Windows',
          'Operating System :: Microsoft :: Windows :: Windows XP',
          'Operating System :: Microsoft :: Windows :: Windows Vista',
          'Operating System :: Microsoft :: Windows :: Windows 7',
          'Operating System :: Microsoft :: Windows :: Windows 8',
          'Operating System :: Microsoft :: Windows :: Windows 8.1',
          'Operating System :: Microsoft :: Windows :: Windows 10',
          'Environment :: Win32 (MS Windows)',
          'Environment :: Console',
      ],
      **kwargs)