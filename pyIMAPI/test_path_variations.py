import pyIMAPI, os

i = pyIMAPI.open("test-rel-dir.iso")

if not i.exists("tmp"):
	if not os.path.isdir("tmp"):
		os.mkdir("tmp")
	t = open("tmp\\test.txt", "w")
	t.write("testing dir")
	t.close()

i.add("tmp")
	
passed = True

tests = [	i.exists("tmp")==True,
			i.exists("\tmp")==False,
			i.exists("\\tmp")==True,
			i.exists("\\\\tmp")==True,
			i.exists("/tmp")==True,
			i.exists("//tmp")==True,
			i.exists("///tmp")==True,
			i.exists("/\\tmp")==True,
			i.exists("tmp\\")==True,
		]
		
for i, test in enumerate(tests):
	passed = passed and test
	print "Test " + str(i) + ": " + str(test)

print "Passed all tests" if passed else "Failed"