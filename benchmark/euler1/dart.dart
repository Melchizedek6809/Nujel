#!/usr/bin/env dart
import 'dart:io';

int testRun(){
    var ret = 0;
    for(var i=0;i<10000000;i++){
            if(((i % 3) == 0) || ((i % 5) == 0)){
                   ret += i;
            }
    }
    return ret;
}

void main(){
     var ret = testRun();
     print('The result is: ${ret}');
}