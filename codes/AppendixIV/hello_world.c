#include <stdio.h>
int main(){
int i;
printf("%p\n",&i);
for(i=0; ;i++) {
	printf("hello world %d\n",i);
	sleep(1);
}
return 0;
}
