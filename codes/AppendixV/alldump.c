#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <error.h>
struct map_entry {
	long int start;
	long int end;
	char *data;
	char desc[15];
	};

char* read_seg(int pid, long int start, long int end) {

	long int size, ret_val;
	char mem_path[15];
	char *data;
	int fmem;

	sprintf(mem_path,"/proc/%d/mem",pid);

	fmem = open(mem_path,O_RDONLY);
	if(fmem < 0 )
		return NULL;

	size = end - start;
	if(size <= 0)
		return NULL;
	data = (char *)malloc(sizeof(char)*size);

	lseek(fmem,start,SEEK_SET);
	ret_val = read(fmem, data, size);
	if(ret_val < size) {
		perror(" [*] incomplete Read");
		return NULL;
	}
	return data;
}

void print_seg(struct map_entry map) {
int i;
long int size;
size = map.end - map.start;
printf("Printing from %ld to %ld \n", map.start, map.end);
for(i=0;i< size;i++)
	printf("%c",map.data[i]);
printf("End printing \n");

}

int write_mem(int pid,struct map_entry **map, struct map_entry **map_own) {
	int i;
	char file_path[15];
	long int start, end,size,ret_val;
	int fmem;

	sprintf(file_path,"/proc/%d/mem",pid);
	fmem = open(file_path,O_RDWR);
	if( fmem <0) {
		perror(" [*] Error opening mem_file for RDWR");
		return -1;
	}

	for(i = 0; (*map)[i].start!=0 && (*map)[i].end!=0; i++) {
	/* Oh yes it works even when stack only is replace back */
//	if(strcmp( (*map_own)[i].desc,"[stack]") != 0)
//		continue;

	printf("old [ %lx, %lx] \t current [%lx, %lx] \n",(*map)[i].start,(*map)[i].end, (*map_own)[i].start,(*map_own)[i].end);
	start = (*map)[i].start;
	end = (*map)[i].end;
	size = end-start;
//	printf("Writing from %lx to %lx\n",start, end);

//	lseek(fmem, start, SEEK_SET);
	lseek(fmem, (*map_own)[i].start, SEEK_SET);
	ret_val = write(fmem, (*map)[i].data, size);
	if(ret_val < size) {
		perror(" [*] Incomplete Write ");
		//close(fmem);
		//return -1;
	}

	}//end for

close(fmem);
return 1;
}

int write_mem2file(char *outfile, struct map_entry **map) {

long int ret_val, size;
int i;
int fout;

fout = open(outfile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
if(fout < 0) {
	perror(" [*] opening outfile for RDWR failed");
	return -1;
}

for(i=0; ; i++) { //as the end marker of map array is with start and end zero we write them too
ret_val = write(fout, &(*map)[i].start, sizeof(long int));
if(ret_val != sizeof(long int)) {
	perror(" [*] start addr Write failed ");
	return -1;
}

ret_val = write(fout, &(*map)[i].end, sizeof(long int));
if(ret_val != sizeof(long int)) {
        perror(" [*] end addr Write failed ");
        return -1;
}

size = (*map)[i].end - (*map)[i].start;
/*ret_val = write(fout, (*map)[i].desc, strlen((*map)[i].desc));
if(ret_val != sizeof(long int)) {
        perror(" [*] Write failed ");
        return -1;
}*/

ret_val = write(fout, (*map)[i].data, size );
if(ret_val != size ) {
        perror(" [*] data Write failed ");
        return -1;
}
if ( (*map)[i].start == 0 && (*map)[i].end == 0 )
	break;
}//end of all map segments 
close(fout);
return 1;
}

int read_mem_from_file(struct map_entry **map, char* outfile) {

	long int start, end, size, ret_val;
	int i, lines=0, fout;
	char *data;
	int size_long_int = sizeof(long int);

	fout = open( outfile, O_RDONLY);

	do { // size findingh loop
	ret_val = read(fout, &start, size_long_int);
	if(ret_val != size_long_int ) {
		perror(" [*] Reading start addre failed");
		return -1;
	}
	ret_val = read(fout, &end, size_long_int);
	if(ret_val != size_long_int) {
		perror(" [*] Reading end addr failed");
		return -1;
	}
	size = end - start;
	if( end == 0 && start == 0)
		break;
	ret_val = lseek(fout, size, SEEK_CUR);
	if(ret_val == -1) {
		perror(" [*] lseek error ");
		return -1;
	}
	lines++;
	} while(1); //end size finding loop
	ret_val = lseek(fout, 0, SEEK_SET);
	*map = (struct map_entry *)malloc(sizeof(struct map_entry)*(lines+1));


	do { //start data copy loop
	ret_val = read(fout, &start, size_long_int);
        if(ret_val != size_long_int ) {
                perror(" [*] Reading start addre failed");
                //return -1;
        }
        ret_val = read(fout, &end, size_long_int);
        if(ret_val != size_long_int) {
                perror(" [*] Reading end addr failed");
                //return -1;
        }

        size = end - start;
        if( end == 0 && start == 0) {
        (*map)[i].start = start;
        (*map)[i].end = end;
        break;
        }

        (*map)[i].data = (char *)malloc(sizeof(char)*size);

        ret_val = read(fout, (*map)[i].data, size);
        if(ret_val != size) {
                perror(" [*] REading data error ");
                return -1;
        }
        (*map)[i].start = start;
        (*map)[i].end = end;

        i++;
        } while(1); //end data copy loop

        printf(" :-) mem structure populated successfully\n");

return 1;
}

//return zero on failure and non-zero on success
int get_maps(int pid,struct map_entry **map) {
	int lines,i;
	char file_path[15], buf[256];
	long int start,end;
	FILE *fmaps;

	sprintf(file_path, "/proc/%d/maps",pid);
	fmaps = fopen(file_path,"r");
	if( !fmaps)
		return -1;
	lines = 0;
	while( fscanf(fmaps, "%[^\n]\n",buf) != EOF) lines++;
	printf(" [*] %d lines found\n",lines);

	*map = (struct map_entry *)malloc(sizeof(struct map_entry)*(lines+1));

	rewind(fmaps);
	i = 0;
	while( fscanf(fmaps, "%lx-%lx%[^\n]\n", &start, &end, buf) != EOF) {
	// Lets try this just by copying only the stack  portion  in map structre
//		if(!strstr(buf,"[stack]"))
//			continue;

		(*map)[i].start = start;
		(*map)[i].end = end;

		(*map)[i].data = read_seg(pid, start, end );
		//(*map)[i].data = "ram";

		if(strstr(buf,"[stack]"))
			strcpy((*map)[i].desc,"[stack]");
		else if(strstr(buf,"[vdso]"))
			strcpy((*map)[i].desc,"[vdso]");

		i++;
	}
	(*map)[i].start = 0;
	(*map)[i].end = 0;
	strcpy((*map)[i].desc,"END");

	fclose(fmaps);
	return lines;
}
int main(int argc, char* argv[]) {
	int pid, i;
	int ret_val=0;
	int seg=0;
	char *opt, *outfile;
	struct map_entry **map, **map_own;

	if(argc < 4){
		printf("Usage: %s <pid> <-d/-r> <outfile>\n",argv[0]);
		return 0;
	}

	pid = atoi(argv[1]);
	opt = argv[2];
	outfile = argv[3];

	map = (struct map_entry **) malloc(sizeof(struct map_entry*));

if( strcmp(opt,"-d")==0) {

	printf(" [!] writing dump file \n");
	ret_val = get_maps(pid, map); //ret_val gives the number of lines read
	seg = ret_val;
	if(ret_val < 0)
		perror(" [*] error reading maps file");
       for(i=0; (*map)[i].start != 0 && (*map)[i].end != 0;i++)
              printf("Start = %lx\tEnd = %lx \t Desc= %s\n",(*map)[i].start, (*map)[i].end, (*map)[i].desc);
	//copy all mem_region from mem file to a outfile
	write_mem2file(outfile, map);
	printf("Finished writng memory to file %s\n",outfile);
} else if( strcmp(opt,"-r")==0) {

	map_own = (struct map_entry **) malloc(sizeof(struct map_entry*));// this is used for lseek only
        ret_val = get_maps(pid, map_own); //ret_val gives the number of lines read
        seg = ret_val;
                if(ret_val < 0)
 			perror(" [*] error reading maps file");
	// read maps file here to 
	printf(" [!] Reading dump file \n");
	read_mem_from_file(map, outfile);
	printf(":D finished reading map structure from %s\n",outfile);
	write_mem(pid, map, map_own);
	printf(":D Writing mem back is successed\n");
} else {
	printf("Usage : %s <pid> <-d/-r> <outfile> \n",argv[0]);
}
	fflush(stdout);
	//read that outfile and replace back all content
	return 0;
}
