local function readAll(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content
end

local function adler32(bv)
      local a = 1
      local b = 0
      for i = 1,#bv do
          a = (a + bv:byte(i,i)) % 65521
          b = (a + b) % 65521
      end
      return a + (b * 65536)
end

print(string.format("%8.8X",adler32(readAll("test-files/r5rs.pdf"))))
