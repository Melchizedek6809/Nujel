function reverseNum(a)
      ret = 0
      while (a > 0)
            ret = (ret * 10) + (a % 10)
            a = floor(a / 10)
      end
      return ret
end

function palindrome(a)
      return (a == reverseNum(a))
end

function startSearch()
      ret = 0
      for a = 1:999
          for b = 1:999
              p = a * b
              if (palindrome(p) && (p > ret))
                 ret = p
              end
          end
      end
      return ret
end

print(string("The biggest product of 2 3-digit numbers that is a palindrome is: ", startSearch()))
