#!/usr/bin/env lua
ret = 0
for i = 1,(10000000-1)
do
        ret = ret + i
end
print("The result is ",ret)