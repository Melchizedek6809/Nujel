try{
    var consoleLog = print ? print : console.log;
}catch(e){
    var consoleLog = console.log;
} // mujs workaround

function multiple_of_3_or_5(a){
    return ((a%3) == 0) || ((a%5) == 0);
}

function test_run(){
    var ret = 0;
    for(var i=0;i<10000000;i++){
        if(multiple_of_3_or_5(i)){
            ret += i;
        }
    }
    return ret;
};
consoleLog("The sum is: " + test_run());
