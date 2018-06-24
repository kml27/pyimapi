import pyIMAPI
print 'imported module'
iso = pyIMAPI.open('test', 'w')
print 'opened test with explicit a+ mode'
print "is_FileSystem(iso)", pyIMAPI.is_FileSystem(iso)
print "is_FileSystem([])", pyIMAPI.is_FileSystem([])

test2 = pyIMAPI.open('test2')
print 'opened optional a+ mode test2'
help(pyIMAPI.FileSystem)
help(pyIMAPI)
help(iso)
help(test2)
print 'calling iso.list()'
iso.list()
iso.close()
test2.close()


