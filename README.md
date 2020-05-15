# README #

This is a Python module to provide a tarfile like object to create ISO 9660 and other optical disc .iso files.

### What is this repository for? ###

#### Quick summary  ####

Create .iso images for optical discs ( data CD or Blu-ray, theoretically Red Book audio) or for mounting to VMs via python scripts. ( I initially created this to work around mounting *WAY TOO MANY* 1.44M floppy images to a DOS VM).

Still early development, but core functionality works. Being alpha/beta stage, the releases still have some debug outputs sometimes. 

#### Latest Version (Please install this if you only plan to use this module)

[v0.4.0b2 on pypi](https://pypi.org/project/pyIMAPI/)

### (If you plan to contribute dev time) How do I get set up? ###

#### Summary of set up
`pip install ./pyIMAPI` from source root

##### Configuration

`%PYTHONPATH%` should be set to the root python directory (e.g. `c:\python27`, `c:\python36`)   

At least VC9 for Python 2.7 is expected http://aka.ms/vcpython27 from a VS command line, I originally used VS2015-2017 to begin development.

`builddistinstallrelease.bat` will use msbuild to build and install the release version w/ pdb (i.e. the build is still debuggable in some form)  

### Contribution guidelines ###

#### Writing tests
  Generally, tests should be .py files that exercise the api rather than .cpp based tests for internal functions. 

##### Code review
  Minimal code review, just provide the basic use case example script and PR.
  
##### Other guidelines
  At this time, the plugin is Windows only. While it might be worth integrating this and the Linux specific iso executable wrapper module, I have not pursued contacting the dev or the python devs to integrate both of these into python core at all (though the idea has crossed my mind). If you wish to invest time in pursuing this, you are welcome to do so, but keep me in the loop and start with the devs responsible for those projects (consider this my consent to use this code toward that end until you get a response from the respective project owner(s)).

### Who do I talk to? ###
bitbucket: ken_long
github: kml27
