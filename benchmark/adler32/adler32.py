def adler32(v):
    a = 1
    b = 0
    for c in v:
        a = (a + c) % 65521
        b = (a + b) % 65521
    return a | (b << 16)

def readFile(filename):
    in_file = open(filename, "rb")
    data = in_file.read()
    in_file.close()
    return data

def adler32sum(filename):
    sum = adler32(readFile(filename))
    print("{0:08X}".format(sum))

adler32sum("test-files/r5rs.pdf")
