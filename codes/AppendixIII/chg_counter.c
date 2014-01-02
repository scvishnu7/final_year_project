#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h> /*for user_regs_struct */

int main(int argc, char *argv[])
{
	pid_t traced_process;
	int addr;
	int value, pid;
	long ins;
	if(argc !=2) {
		printf("Usage: %s <pid to be traced>\n",argv[0]);
		return 0;
	}
	pid= atoi(argv[1]);
	
	traced_process = atoi(argv[1]);
	ptrace(PTRACE_ATTACH, traced_process, NULL, NULL);
	printf("Enter the variable address in Hex: ");
	scanf("%x",&addr);
	value = ptrace(PTRACE_PEEKDATA,pid, addr, NULL);
	printf(" Current Variable value is  %d \n", value);
	printf(" Enter new value : ");
	scanf("%d", &value);
	ptrace(PTRACE_POKEDATA, pid, addr, value);
	ptrace(PTRACE_DETACH, traced_process, NULL, NULL);
	return 0;
}
