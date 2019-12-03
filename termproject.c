#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ls2mod.h"

#define nl() { puts(""); }

void execPwd() {
    int f = fork();
    
    if(f == 0) { //자식 프로세스
	execlp("pwd", "pwd", NULL);
	exit(0);
    } else {
	wait(NULL);
    }
}

void execLs() {
    int f = fork();

    if(f == 0) { //자식 프로세스
	//execlp("ls", "ls", NULL);
	char* temp_av[] = { "ls", NULL };
	my_ls(1, temp_av);
	exit(0);
    } else {
	wait(NULL);
    }
}

void showWindow() {
    execPwd();
    nl();
    execLs();
    nl();
}

//char*로된 문자열을 char**로 변환
char** stringTo2darr(char* src, char** dst) {
    char *delim = " \n";
    int n = 0;

    for (char *p = strtok (src, delim); n < BUFSIZ && p; p = strtok (NULL, delim)) {
	dst[n++] = p;
    }
    dst[n++] = NULL;

    return dst;
}

int main(int ac, char* av[]) {
    
    char input[BUFSIZ];

    while(1) {
	showWindow();

	printf("linux explorer: ");
	fgets(input, BUFSIZ, stdin);
	
	int f = fork();
	if(f == 0) {
	    char* input2d[BUFSIZ];
	    stringTo2darr(input, input2d);
	    execvp(input2d[0], input2d);
	    exit(0);
	} else {
	    wait(NULL);
	}
    }

    return 0;
}
