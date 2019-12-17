#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define RED "\e[1;31m"
#define DEF "\e[0m"

int main() {
	creat("asda.txt", 0644);
	return 0;
}
