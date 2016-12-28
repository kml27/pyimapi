import pyIMAPI2FS
print 'imported module'
iso = pyIMAPI2FS.open('test', 'a+')
print 'opened test with explicit a+ mode'
print "is_FileSystem(iso)", pyIMAPI2FS.is_FileSystem(iso)
print "is_FileSystem([])", pyIMAPI2FS.is_FileSystem([])

test2 = pyIMAPI2FS.open('test2')
print 'opened optional a+ mode test2'
help(pyIMAPI2FS.FileSystem)
help(pyIMAPI2FS)
help(iso)
help(test2)
print 'calling iso.list()'
iso.list()


