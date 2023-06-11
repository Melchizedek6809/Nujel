#!/usr/bin/env lua
local function bench ()
   local ret = 0
   for i = 1,(10000000-1)
   do
      ret = ret + i
   end
   return ret
end

print("The result is ", bench())
