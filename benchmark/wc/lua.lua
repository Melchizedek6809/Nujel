file = io.open ("benchmark/bib.txt", "r")

con = file:read("*a")

local nl = 0;
local nw = 0;
local nc = con:len();
local inWord = false;

for i = 1, con:len() do
    local c = con:sub(i,i)
    if (c == " ") then
        inWord = false;
    else
        if (c == "\n") then
            inWord = false;
            nl = nl + 1;
        else
            if (not inWord) then
                nw = nw + 1
            end
            inWord = true
        end
    end
end

print ("Lines: " .. nl)
print ("Words: " .. nw)
print ("Characters: " .. nc)

io.close(file)