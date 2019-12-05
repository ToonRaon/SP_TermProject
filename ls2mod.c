#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#define FALSE 0
#define TRUE 1

#define DEBUG 0

#define DEFAULT "\e[0m"
#define BLACK "\e[0;30m"
#define RED "\e[0;31m"
#define GREEN "\e[0;32m"
#define ORANGE "\e[0;33m"
#define BLUE "\e[0;34m"
#define PURPLE "\e[0;35m"
#define CYAN "\e[0;36m"
#define LIGHT_GRAY "\e[0;37m"
#define DARK_GRAY "\e[1;30m"
#define LIGHT_RED "\e[1;31m"
#define LIGHT_GREEN "\e[1;32m"
#define YELLOW "\e[1;33m"
#define LIGHT_BLUE "\e[1;34m"
#define LIGHT_PURPLE "\e[1;35m"
#define LIGHT_CYAN "\e[1;36m"
#define WHITE "\e[1;37m"


int isReverse = FALSE;
int isTime = FALSE;


struct file_information {
	char* filename;
	struct stat* info_p;
	struct file_information* next;
};

struct file_information* header;

void do_ls(char[]);
void showAll();



void setTextColor(char* color) {
    printf("%s", color);
}

void printList() {
	struct file_information* node = header->next;

	while(node != NULL) {
		printf("%s ", node->filename);
		node = node->next;
	}
	puts("");
}

struct file_information* makeNode(char* name, struct stat* info, struct file_information* next) {
	struct file_information* temp;
	temp = (struct file_information*)malloc(sizeof(struct file_information));
	temp->filename = name;
	if(info != NULL) {
		struct stat* temp_info = (struct stat*)malloc(sizeof(struct stat));
		memcpy(temp_info, info, sizeof(struct stat));
		temp->info_p = temp_info;
	} else {
		temp->info_p = NULL;
	}
	temp->next = next;
	
	//printf("%s 노드를 생성하였습니다. st_mtime: %d\n", temp->filename, info != NULL ? (int)temp->info_p->st_mtime : -1);
	
	return temp;
}

int paramsCheck(int n, char* str[]) {
	int count = 0;
	for(int i = 1; i < n; i++) {
		if(strstr(*(str + i), "-")) { //파라미터 중에 -가 들어가는 파라미터 개수를 새서 마지막에 반환해준다
			count++;

			if(strstr(*(str + i), "t")) {
				isTime = TRUE;
			}
			if(strstr(*(str + i), "r")) {
				isReverse = TRUE;
			}
		}
/*
		if(strstr(*(str + i), "-tr") || strstr(*(str + i), "-rt")) {
			isReverse = TRUE;
			isTime = TRUE;
		} else if(strstr(*(str + i), "-t")) {
			isTime = TRUE;
		} else if(strstr(*(str + i), "-r")) {
			isReverse = TRUE;
		}
*/
	}

	return count;
}

void init() {
	//init header
	header = makeNode(NULL, NULL, NULL);
}

void my_ls(int ac, char* av[]) {
    int optionCount = paramsCheck(ac, av); //check options and return how many options

    if(ac - optionCount == 1 ) { //그냥 ac == 1하면 ls -rt <-와 같은 경우를 못 잡아냄
        do_ls(".");
    } else {
        while(--ac) {
    	++av;
    	if(*av[0] == '-') { //ignore that start with dash(-)
    	    continue;
    	}

	    printf("%s:\n", *av);
	    do_ls(*av);
	}
    }
}

char* gid_to_name(gid_t gid) {
	struct group *getgrid(), *grp_ptr;
	static char numstr[10];
	if((grp_ptr = getgrgid(gid)) == NULL) {
		sprintf(numstr, "%d", gid);
		return numstr;
	} else
		return grp_ptr->gr_name;
}

char* uid_to_name(uid_t uid) {
	struct passwd *getpwuid(), *pw_ptr;
	static char numstr[10];

	if((pw_ptr = getpwuid(uid)) == NULL) {
		sprintf(numstr, "%d", uid);
		return numstr;
	} else 
		return pw_ptr->pw_name;
}

void mode_to_letters(int mode, char str[]) {
	strcpy(str, "----------");

	if(S_ISDIR(mode)) str[0] = 'd';
	if(S_ISCHR(mode)) str[0] = 'c';
	if(S_ISBLK(mode)) str[0] = 'b';
	if(S_ISLNK(mode)) str[0] = 'l'; //심볼릭 링크

	if(mode & S_IRUSR) str[1] = 'r';
	if(mode & S_IWUSR) str[2] = 'w';
	if(mode & S_IXUSR) str[3] = 'x';

	if(mode & S_IRGRP) str[4] = 'r';
	if(mode & S_IWGRP) str[5] = 'w';
	if(mode & S_IXGRP) str[6] = 'x';

	if(mode & S_IROTH) str[7] = 'r';
	if(mode & S_IWOTH) str[8] = 'w';
	if(mode & S_IXOTH) str[9] = 'x';
}

void show_file_info(char* filename, struct stat* info_p) {
	char *uid_to_name(), *ctime(), *gid_to_name(), *filemode();
	void mode_to_letters();
	char modestr[11];
	
	if(info_p->st_mode & 0111) {
	    setTextColor(GREEN);
	}
	if(S_ISDIR(info_p->st_mode)) {
	    setTextColor(BLUE);
	}
	if(S_ISLNK(info_p->st_mode)) {
	    setTextColor(CYAN);
	}

	mode_to_letters(info_p->st_mode, modestr);
//	printf("%o ", info_p->st_mode);
	printf("%s", modestr);
	printf("%4d ", (int) info_p->st_nlink);
	printf("%-8s ", uid_to_name(info_p->st_uid));
	printf("%-8s ", gid_to_name(info_p->st_gid));
	printf("%-8ld ", (long)info_p->st_size);
	printf("%.12s ", 4+ctime(&info_p->st_mtime));
#if DEBUG
	printf("%d ", (int)info_p->st_mtime);
#endif
	printf("%s \n", filename);

	setTextColor(DEFAULT);

}

void addFilearr(char* filename, struct stat* info_p) {
	if(header->next == NULL) { // first added file information to linked list
		header->next = makeNode(filename, info_p, NULL);
#if DEBUG
		printf("Added file to linked list %s -> ", filename);
		printList();
#endif
	} else {
		if(isTime && isReverse) { //오름차순 정렬 (오래된 파일이 제일 위)
			struct file_information* curNode = header->next;
			struct file_information* prevNode = header;
                        while((curNode != NULL) && (curNode->info_p->st_mtime < info_p->st_mtime)) {
				prevNode = curNode;
                                curNode = curNode->next;
                        } //while문 끝났을 때, 새로운 노드는 prevNode와 curNode 사이에 넣어야 한다

                        prevNode->next = makeNode(filename, info_p, curNode);
		} else if(isTime && !isReverse) { //내림차순 정렬 (최근 파일이 제일 위)
			struct file_information* curNode = header->next;
			struct file_information* prevNode = header;
			while((curNode != NULL) && (curNode->info_p->st_mtime > info_p->st_mtime)) {
				prevNode = curNode;
				curNode = curNode->next;
			}  //while문 끝났을 때, 새로운 노드는 prevNode와 curNode 사이에 넣어야 한다

			prevNode->next = makeNode(filename, info_p, curNode);
		} else { //정렬 안함
			header->next = makeNode(filename, info_p, header->next);
		}
#if DEBUG
			printf("Added file to linked list %s -> ", filename);
			printList();
#endif
	}
}

void dostat(char* filename) {
	struct stat info;
	if(lstat(filename, &info) == -1)
		perror(filename);
	else {
		//show_file_info(filename, &info); //이거 큐 추가로 바꾸기
		addFilearr(filename, &info);
	}
}

void showAll() {
	struct file_information* node = header->next;

	int file_no = 0;
	while(node != NULL) {
	   printf("%d: ", file_no++); 
	   show_file_info(node->filename, node->info_p);		
	   node = node->next;
	}
	puts("");
}

void do_ls(char dirname[]) {
	DIR* dir_ptr;
	struct dirent* direntp;

	init(); //init header as null

	if((dir_ptr = opendir(dirname)) == NULL) {
		fprintf(stderr, "ls2mod: cannot open %s\n", dirname);
	} else {
		chdir(dirname);
		while((direntp = readdir(dir_ptr)) != NULL) {
			dostat(direntp->d_name);
		}
		showAll();
		closedir(dir_ptr);
		chdir("..");

//		showAll();
	}
}
