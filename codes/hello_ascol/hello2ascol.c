#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>

int main(int argc, char* argv[]) {

FILE *maps_file;
int mem_file;
char map_path[20];
char mem_path[20];
int pid;
unsigned long start, end, data;
int size,i;
char ch,cha,  *buf;

if(argc < 2){
	printf("Usage: %s <pid of 'hello_world' program>\n",argv[0]);
	return 0;
	}
pid= atoi(argv[1]);

sprintf(map_path, "%s/%d/%s","/proc",pid,"maps");
sprintf(mem_path, "%s/%d/%s","/proc",pid,"mem");

maps_file = fopen(map_path,"r");
mem_file = open(mem_path,O_RDWR);

fscanf(maps_file,"%lx-%lx",&start, &end);
printf("Code segment = [%lX , %lX] \n", start, end);
fclose(maps_file); /* lets close the maps file */

lseek(mem_file, start, SEEK_SET);
char ch1,ch2,ch3,ch4, ch5, ch6;
size = end - start;
for( i=0;i<size ;i ++) {
	read(mem_file, &ch,1);
	ch6 = ch5;
	ch5 = ch4;
	ch4 = ch3;
	ch3 = ch2;
	ch2 = ch1;
	ch1 = ch;
	if((ch6 == 'h') && (ch5=='e') &&(ch4 =='l')&& (ch3=='l') && (ch2 == 'o') && (ch1 == ' ')){
		printf("\"hello  \" found, Lets change the remaning 5 char to ASCOL\n");
		int retval = write(mem_file,"ASCOL",5);
			if( retval <= 0) {
				printf("mem file write error. run with sudo.\n");
				close(mem_file);
				return -1;
		}
		break;
	}
	//printf("%c",ch);

}
close(mem_file);
printf("Code changed   \n");
return 0;
}
