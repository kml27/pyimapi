
	Create CD, DVD, DVD Dual Layer, and BluRay ISO images under Windows directly from python.

		* No physical drives or additonal software required
		* Light-weight
		* Free 
		* Open source

	pyIMAPI exposes built-in Windows Image Mastering API functionality. 
	
	Support has also been added to mount ISO images using PowerShell Mount-DiskImage. Currently, pyIMAPI only uses ISO9660 mode when creating ISOs. Other IMAPI functionality like Joliet format and Redbook Audio is not currently supported, but can be added.  

	Release Notes for v0.1.0a19
	
	## What's New
		* Support for DVD, DVDDL (DVD Dual Layer), and BluRay physical media formats (allows creating larger iso images)
		* Mount Images using Powershell
		* "Extract" files from mounted images 
	## Fixes
			* General Misc stability fixes
			* More testing

	## Example Usage

	``` from __future__ import print_function
	import pyIMAPI

	print("writing test file")
	t = open("test.txt", "w")
	t.write("this test works")
	t.close()

	print("creating default iso (BluRay)")
	o = pyIMAPI.open("test.iso", disk_type="BluRay")
	o.add("test.txt")
	o.close()

	print("opening created iso")
	i = pyIMAPI.open("test.iso", "r")

	print("extracting file")
	i.extract("test.txt", "./test-extracted.txt")
	i.close()

	print("opening source and extracted files")
	t = open("test.txt")
	t2 = open("test-extracted.txt")


	[Source](https://bitbucket.org/ken_long/pyIMAPI2FS)
