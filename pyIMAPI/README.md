
	Create CD, DVD, DVD Dual Layer, and BluRay ISO images under Windows directly from python.

		* No physical drives or additonal software required
		* Light-weight
		* Free 
		* Open source

	pyIMAPI exposes built-in Windows Image Mastering API functionality. 
	
	Support has also been added to mount ISO images using PowerShell Mount-DiskImage. Currently, pyIMAPI only uses ISO9660 mode when creating ISOs. Other IMAPI functionality like Joliet format and Redbook Audio is not currently supported, but can be added.  

	Release Notes for v0.4.0b2
	
	## What's New
		* Support for DVD, DVDDL (DVD Dual Layer), and BluRay physical media formats (allows creating larger iso images)
	 	* Mount Images using Powershell in Windows 8 and later
		* "Extract" files from mounted images 
		* Wheel x64 Distribution for Python 3.4, 3.5, 3.6, and 3.7 
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

	print("creating default iso (\"CD\")")
	o = pyIMAPI.open("test.iso")
	o.add("test.txt")
	o.close()

	disk_types = ["CD", "DVD", "DVDDL", "BluRay"]

	for dt in disk_types:
		specific_physical_media_test_filename = dt+"-test.txt"
		t = open(specific_physical_media_test_filename, "w")
		t.write("this "+ dt +" test works")
		t.close()

		o = pyIMAPI.open(dt+"-test.iso", disk_type=dt)
		o.add(specific_physical_media_test_filename)
		o.close()

	print("opening created iso")
	i = pyIMAPI.open("test.iso", "r")

	print("extracting file")
	i.extract("test.txt", "./test-extracted.txt")
	i.close()

	print("opening source and extracted files")
	t = open("test.txt")
	t2 = open("test-extracted.txt")

	print("comparing files")
	for line in t:
		extracted = t2.read()
		if extracted != line:
			print("error files don't match")
			quit()

	print("extracted file matches original") ```


	[Source](https://bitbucket.org/ken_long/pyIMAPI2FS)
