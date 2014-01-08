#include <stdio.h>

int main() {

	char name1[10], name2[10], name3[10];
	printf("1 >> Enter the 1st name: "); 
	scanf("%s", name1);
	
	printf("2 >> Enter the 2nd name: ");
	scanf("%s", name2);
	
	printf("3 >> Enter the 3rd name: ");
	scanf("%s", name3);
	printf("name1 = %s name2 = %s name3 = %s \n", name1, name2, name3);
	
	return 0;
}
