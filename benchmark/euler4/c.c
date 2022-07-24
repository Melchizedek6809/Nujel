#include <stdio.h>

int reverseNum(int a){
	int ret = 0;
	for(;a>0;a/=10){
		ret = (ret * 10) + (a % 10);
	}
	return ret;
}

int palindromeP(int a){
	return a == reverseNum(a);
}

int startSearch() {
	int ret = 0;
	for(int a=0;a<1000;a++){
		for(int b=0;b<1000;b++){
			int p = a * b;
			if(palindromeP(p) && (p > ret)){
				ret = p;
			}
		}
	}
	return ret;
}

int main(int argc, char *argv[]){
	printf("The biggest product of 2 3-digit numbers that is a palindrome is: %i\n", startSearch());
	return 0;
}
