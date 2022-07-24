#!/usr/bin/env dart
import 'dart:io';


int reverseNum(a){
    int ret = 0;
    for(;a>0;a = (a~/10)){
        ret = ((ret * 10) + (a % 10)).floor();
    }
    return ret;
}

bool palindromeP(a){
  return a == reverseNum(a);
}
int startSearch() {
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

void main(){
  var ret = startSearch();
  print("The biggest product of 2 3-digit numbers that is a palindrome is: ${ret}");
}
