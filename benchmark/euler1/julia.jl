function testCalc()
    ret = 0
    for i in 1:(10000000-1)
        if (((i % 3) == 0) || ((i % 5) == 0))
            ret = ret + i
        end
    end
    return ret
end
print(string("The sum is: ", testCalc()))
