#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ls2mod.h"

#define nl() { puts(""); }

#define BLACK 0;30
#define RED 0;31
#define GREEN 0;32
#define ORANGE 0;33
#define BLUE 0;34
#define PURPLE 0;35
#define CYAN 0;36
#define LIGHT_GRAY 0;37
#define DARK_GRAY 1;30
#define LIGHT_RED 1;31
#define LIGHT_GREEN 1;32
#define YELLOW 1;33
#define LIGHT_BLUE 1;34
#define LIGHT_PURPLE 1;35
#define LIGHT_CYAN 1;36
#define WHITE 1;37

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
