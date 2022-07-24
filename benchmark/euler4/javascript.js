try{
    var consoleLog = print ? print : console.log;
}catch(e){
    var consoleLog = console.log;
} // mujs workaround

function reverseNum(a){
    var ret = 0;
    for(;a>0;a = (a/10)|0){
        ret = (ret * 10) + (a % 10);
    }
    return ret;
}

function palindromeP(a){
    return a == reverseNum(a);
}

function startSearch() {
    var ret = 0;
    for(var a=0;a<1000;a++){
        for(var b=0;b<1000;b++){
            var p = a * b;
            if(palindromeP(p) && (p > ret)){
                ret = p;
            }
        }
    }
    return ret;
}

consoleLog("The biggest product of 2 3-digit numbers that is a palindrome is: " + startSearch());
