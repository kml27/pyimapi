#Example Use Case #3
#Open an existing ISO for reading

import pyIMAPI2FS, os

outfile = "test.iso"

iso_file = pyIMAPI2FS.open(outfile, mode="r")

iso_file.list()

iso_file.close()
