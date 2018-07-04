from __future__ import print_function
import pyIMAPI, os

print("writing test file")
t = open("test.txt", "w")
t.write("this test works")
t.close()

os.mkdir("tmp")
os.mkdir("tmp2")

t = open("tmp\\tmp-test.txt","w")
t.write("example for adding a subdirectory by abs local path")
t.close()

t = open("tmp2\\tmp2-test.txt","w")
t.write("example for adding a subdirectory by relative local path")
t.close()


print("creating default iso (\"CD\")")
o = pyIMAPI.open("test.iso")
o.add("test.txt")
o.add(os.getcwd()+"\\tmp")
o.add("tmp2")
o.add("tmp2", False)
#this next call should fail with a dir does not exist
o.add("tmp3")
o.add("test2.txt")
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

print("extracted file matches original")

disk_types = ["CD", "DVD", "DVDDL", "BluRay"]

for dt in disk_types:
	specific_physical_media_test_filename = dt+"-test.txt"
	t = open(specific_physical_media_test_filename, "w")
	t.write("this "+ dt +" test works")
	t.close()

	o = pyIMAPI.open(dt+"-test.iso", disk_type=dt)
	o.add(specific_physical_media_test_filename)
	o.close()
