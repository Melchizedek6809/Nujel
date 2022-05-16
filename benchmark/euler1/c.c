#include <stdio.h>

int main(int argc, char *argv[]){
	long long int ret = 0;
	for(int i=0;i<10000000;i++){
		if(((i % 3) == 0) || ((i % 5) == 0)){
			ret += i;
		}
	}
	printf("This sum is: %lli\n", ret);
	return 0;
}
