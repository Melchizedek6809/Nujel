#include <stdio.h>

int main(int argc, char *argv[]){
	long long int ret = 0;
	for(int i=0;i<10000000;i++){
		ret += i;
	}
	printf("This results in: %lli\n", ret);
	return 0;
}
