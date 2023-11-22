"use strict";

try{
    var consoleLog = print ? print : console.log;
}catch(e){
    var consoleLog = console.log;
} // mujs workaround

function testRun(){
    var ret = 0;
    for(var i=0;i<10000000;i++){
        ret += i;
    }
    consoleLog("The result is: " + ret);
}
testRun();
