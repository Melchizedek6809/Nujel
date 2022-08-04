def testRun()
    var ret = 0
    for i: 0 .. (10000000-1)
        ret += i
    end
    print("The result is: " .. ret)
end

testRun()
