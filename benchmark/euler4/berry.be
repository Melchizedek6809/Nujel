def reverseNum(a)
    var ret = 0
    while a>0
        ret = (ret * 10) + (a % 10)
        a = (a/10)
    end
    return ret
end

def palindromeP(a)
    return a == reverseNum(a)
end

def startSearch()
    var ret = 0
    for a:0 .. 999
        for b:0 .. 999
            var p = a * b
            if(palindromeP(p) && (p > ret))
                ret = p
            end
        end
    end
    return ret
end

print("The biggest product of 2 3-digit numbers that is a palindrome is: " .. startSearch())
