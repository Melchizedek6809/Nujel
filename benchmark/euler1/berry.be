def test_run()
    var ret = 0
    for i:0 .. (10000000-1)
        if (((i%3) == 0) || ((i%5) == 0))
            ret += i
        end
    end
    return ret
end
print("The sum is: " .. test_run())
