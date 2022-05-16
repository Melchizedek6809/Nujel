function testCalc()
    ret = 0
    for i in 1:(10000000-1)
        ret = ret + i
    end
    return ret
end
print(string("This results in: ", testCalc()))
