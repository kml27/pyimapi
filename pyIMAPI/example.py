from __future__ import print_function
import pyIMAPI

print("writing test file")
t = open("test.txt", "w")
t.write("this test works")
t.close()

print("creating default iso (BluRay)")
o = pyIMAPI.open("test.iso")
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

print("comparing files")
for line in t:
	extracted = t2.read()
	if extracted != line:
		print("error files don't match")
		quit()

print("extracted file matches original")