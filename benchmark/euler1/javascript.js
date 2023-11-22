"use strict";

try{
    var consoleLog = print ? print : console.log;
}catch(e){
    var consoleLog = console.log;
} // mujs workaround

function test_run(){
    var ret = 0;
    for(var i=0;i<10000000;i++){
        if(((i%3) == 0) || ((i%5) == 0)){
            ret += i;
        }
    }
    return ret;
};
consoleLog("The sum is: " + test_run());
