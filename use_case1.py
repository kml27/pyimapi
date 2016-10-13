#Example Use Case #1
#Create an ISO file

import pyIMAPI2FS, os

outfile = "test.iso"

#modes =["r", "w", "a", "r+"]
#default to r+
#"r"
#Opens for reading. If the file does not exist or cannot be found, the fopen call fails.
#"w"
#Opens an empty file for writing. If the given file exists, its contents are destroyed.
#"a"
#Opens for writing at the end of the file (appending) without removing the end-of-file (EOF) marker before new data is written to the file. Creates the file if it does not exist.
#"r+"
#Opens for both reading and writing. The file must exist.


iso_file = pyIMAPI2FS.open(outfile, mode="w")

if type(iso_file) is pyIMAPI2FS.FileSystem:
    print("Opened IMAPI2 ISO filesystem")

iso_file.add(infile)
#file added as "/test/<infile>"

iso_file.close()
