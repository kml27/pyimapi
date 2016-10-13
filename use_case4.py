#Example Use Case #4
#Extract a file from an existing ISO

import pyIMAPI2FS, os

outfile = "test.iso"

iso_file = pyIMAPI2FS.open(outfile, mode="r")

iso_file.extract("test", path="")

iso_file.close()
