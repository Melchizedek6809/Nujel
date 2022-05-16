#!/usr/bin/env lua
ret = 0
for i = 1,(10000000-1)
do
        if (((i % 3) == 0) or ((i % 5) == 0)) then
               ret = ret + i
        end
end
print("The sum is ",ret)