def reverseNum(a):
    ret = 0
    while a > 0:
        ret = (ret * 10) + (a % 10)
        a = a//10
    return ret

def palindromeP(a):
    return a == reverseNum(a)

def startSearch():
    ret = 0
    for a in range(1000):
        for b in range(1000):
            p = a * b
            if (palindromeP(p) and (p > ret)):
                ret = p
    return ret

print("The biggest product of 2 3-digit numbers that is a palindrome is: ", startSearch())
