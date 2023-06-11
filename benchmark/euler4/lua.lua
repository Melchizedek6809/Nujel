-- The biggest product of 2 3-digit numbers that is a palindrome
-- https://projecteuler.net/problem=4
local function reverseNum (a)
      local ret = 0
      while (a > 0) do
            ret = (ret * 10) + (a % 10)
            a = math.floor(a / 10)
      end
      return ret
end

local function palindrome (a)
      return (a == reverseNum(a))
end

local function startSearch ()
      local ret = 0
      for a = 0,1000 do
          for b = 0,1000 do
              local p = a * b
              if (palindrome(p) and (p > ret)) then
                 ret = p
              end
          end
      end
      return ret
end

print("The biggest product of 2 3-digit numbers that is a palindrome is: ", startSearch())
