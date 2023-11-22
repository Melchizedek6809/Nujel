"use strict";

try{
    var consoleLog = print ? print : console.log;
}catch(e){
    var consoleLog = console.log;
} // mujs workaround

consoleLog("Hello");
