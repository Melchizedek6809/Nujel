#!/usr/bin/env dart
import 'dart:io';

void main(){
     var ret = 0;
     for(var i=0;i<10000000;i++){
             ret += i;
     }
     print('The result is: ${ret}');
}