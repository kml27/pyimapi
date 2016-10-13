#Example Use Case #2
#Create an ISO file with directory

import pyIMAPI2FS, os

outfile = "test.iso"

iso_file = pyIMAPI2FS.open(outfile, mode="w")

iso_file.mkdir("test")
iso_file.getcwd()
#displays root, e.g. /

iso_file.chdir("test")
iso_file.getcwd()

iso_file.add(infile)
#file added as "/test/<infile>"

iso_file.close()
