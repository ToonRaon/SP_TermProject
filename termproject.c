#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

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

#define tt() do{ puts("!@#$%^&*()_+"); } while(0)

int isReverse = FALSE;
int isTime = FALSE;


struct file_information {
	char* filename;
	struct stat* info_p;
	struct file_information* next;
};
struct file_information* header = NULL;

void do_ls(char[]);
void showAll();

struct hardlinkNode {
	int inode;
	char fname[512];
};
struct hardlinkNode* hardlinkList = NULL;
int hardlinkListSize = 0;


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
	}

	return count;
}

void freeNode(struct file_information* n) {
	if(n != NULL) {
		freeNode(n->next);
		free(n);
	}
}

void init() {
	//init header
	if(header != NULL) {
		freeNode(header);
	}
	
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

void getPwdAsString(char* str) {
	int f = fork();
//	int p[2];
//	pipe(p);

	if(f == 0) {
//		dup2(p[1], 1);
		close(1);
		creat("pwdtmp", 0644);
		execlp("pwd", "pwd", NULL);
	} else {
		wait(NULL);
		
		FILE* fp = fopen("pwdtmp", "r");
		fscanf(fp, "%s", str);
		fclose(fp);
		remove("pwdtmp");
	}
}

int getFileLineLength(char* fname) {
	int n = 0;
	int f = fork();

	if(f == 0) {
		close(1);
		creat("tmppp", 0644);
		execlp("wc", "wc", "-l", fname, NULL);
	} else {
		wait(NULL);

		FILE* fp = fopen("tmppp", "r");
		fscanf(fp, "%d ", &n);
		fclose(fp);
		remove("tmppp");

		return n;
	}
}

//inode, 파일명, 링크 수를 파라미터로 넣으면 같은 하드링크 파일들을 찾아준다
void showHardLink(int inode, char* fname, int hardlinkCounter) {
	char path[BUFSIZ]; //경로만
	char pathfname[BUFSIZ]; //경로 + 파일명
	getPwdAsString(path);
	strcpy(pathfname, path);
	strcat(pathfname, "/");
	strcat(pathfname, fname);

	char hardlinkFile[BUFSIZ];
	strcpy(hardlinkFile, getenv("HOME"));
	strcat(hardlinkFile, "/");
	strcat(hardlinkFile, ".hardlink");

	if(hardlinkList == NULL) { //hardlinkList 처음엔 초기화 한 번 해주고
		int n = getFileLineLength(hardlinkFile); //마지막 한 줄은 그냥 빈줄이라 - 1
		hardlinkList = (struct hardlinkNode*)malloc(sizeof(struct hardlinkNode) * n);
	
		FILE* fp = fopen(hardlinkFile, "r");
		for(int i = 0; i < n; i++) {
			fscanf(fp, "%d %s\n", &hardlinkList[i].inode, hardlinkList[i].fname);
		}

		hardlinkListSize = n;
	}

	int foundLinkCounter = 1; //지금까지 찾은 하드 링크 개수 (본인 포함이므로 1로 시작)
	for(int i = 0; i < hardlinkListSize && foundLinkCounter < hardlinkCounter; i++) {
		if(hardlinkList[i].inode == inode && strcmp(hardlinkList[i].fname, pathfname) != 0) { //inode 같고 파일명 다르면 -> 하드링크 형제 찾은 거
			foundLinkCounter++;
			printf("<-> %s ", hardlinkList[i].fname);
		}
	}
}


//.hardlink 파일에 기존에 저장된 inode, filename을 가진 게 없으면 새로 추가한다
void addHardlinkToFile(int inode, char* filename) {
	char hardlinkFile[strlen(getenv("HOME")) + strlen("/.hardlink") + 1];
	strcpy(hardlinkFile, getenv("HOME"));
	strcat(hardlinkFile, "/.hardlink");

	FILE* fp = fopen(hardlinkFile, "a+"); //읽기 및 append 모드
	
	int n = getFileLineLength(hardlinkFile);
	for(int i = 0; i < n; i++) {
		char buf[255];
		fgets(buf, 255, fp);
		int a;
		char b[255];
		sscanf(buf, "%d %s", &a, b);
//		printf("buf: %d %s\n", a, b);

		if(a == inode && strcmp(b, filename) == 0) { //이미 같은 inode와 파일명을 가진게 있으면 스킵
			return;
		}
	}

	fprintf(fp, "%d %s\n", inode, filename);
	fclose(fp);
}

//dostat과 비슷. 현재 파일(폴더일수도 있음)을 검사하여 폴더이면 checkDir, 파일이면 하드링크인지 확인
void checkFile(char* filename) {
//	printf("checkFile: %s 조사 중...\n", filename);
//	sleep(1);

	void checkDir(char*);

	struct stat info;

	if(lstat(filename, &info) == -1)
		perror(filename);
	else {
		if(S_ISDIR(info.st_mode)) { //폴더면 checkDir
			checkDir(filename);
		} else { //파일이면 하드링크인지 검사
			if((int)info.st_nlink > 1) { //하드링크
				addHardlinkToFile(info.st_ino, filename); //.hardlink 파일에 정보 추가
			}
		}
	}
}


//do_ls와 비슷. dirname 폴더를 뒤져서 파일이면 하드링크인지 검사하고, 폴더면 재귀적으로 다시 checkDir한다
void checkDir(char* dirname) {
//	printf("checkDir: %s 조사 중...\n", dirname);
//	sleep(1);

	DIR* dir_ptr;
	struct dirent* direntp;

	if((dir_ptr = opendir(dirname)) == NULL) {
		fprintf(stderr, "ls2mod: cannot open %s\n", dirname);
	} else {
		while((direntp = readdir(dir_ptr)) != NULL) {
			char* fname = direntp->d_name;
			if(strcmp(fname, ".") == 0 || strcmp(fname, "..") == 0) { //무한 루프 안 돌게
				continue;
			}

			char path[strlen(dirname) + strlen("/") + strlen(fname) + 1]; //경로 + 파일명
			strcpy(path, dirname);
			strcat(path, "/");
			strcat(path, fname);
			checkFile(path);
		}
		closedir(dir_ptr);
	}
}


//.hardlink 파일을 업데이트해주는 함수 (쓰레드에 의해 돌아감)
void updateHardlinkFile() {
	checkDir(getenv("HOME"));
}

void show_file_info(char* filename, struct stat* info_p) {
	char *uid_to_name(), *ctime(), *gid_to_name(), *filemode();
	void mode_to_letters();
	char modestr[11];
	
	if(info_p->st_mode & 0111) {
	    setTextColor(GREEN);
	}
	if(S_ISDIR(info_p->st_mode)) {
	    setTextColor(LIGHT_BLUE);
	} else if((int) info_p->st_nlink > 1) { //디렉토리가 아니면서 link 수가 2이상이면 하드링크
		setTextColor(PURPLE);
	}
	if(S_ISLNK(info_p->st_mode)) {
	    setTextColor(CYAN);
	}

	mode_to_letters(info_p->st_mode, modestr);
	printf("%s", modestr);
	printf("\t%d ", (int)info_p->st_ino);
	printf("%4d ", (int)info_p->st_nlink);
	printf("%-8s ", uid_to_name(info_p->st_uid));
	printf("%-8s ", gid_to_name(info_p->st_gid));
	printf("%-8ld ", (long)info_p->st_size);
	printf("%.12s ", 4+ctime(&info_p->st_mtime));
	printf("%s ", filename);
	if(!S_ISDIR(info_p->st_mode) && (int)info_p->st_nlink > 1) { //하드링크
		showHardLink((int)info_p->st_ino, filename, (int)info_p->st_nlink);
	}

	//심볼릭 링크인 경우에 파일명 옆에 -> 실제 파일명도 표시
	char buf[BUFSIZ];
	readlink(filename, buf, BUFSIZ);
	if(S_ISLNK(info_p->st_mode)) printf("-> %s", buf);
	printf("\n");

	setTextColor(DEFAULT);
	fflush(stdout);
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

	int file_no = 1;
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
		while((direntp = readdir(dir_ptr)) != NULL) {
			dostat(direntp->d_name);
		}
		showAll();
		closedir(dir_ptr);
	}
}

//header로부터 k번째에 있는 file_information 반환. 헤더가 0번
struct file_information* getFileInfoFromHeader(int k) {
	struct file_information* node = header;
	while(k--) {
		if(node->next != NULL) {
			node = node->next;
		}
	}

	return node;
}


// ============================== ls2mod.c 끝 ============================== //
// ============================== ls2mod.c 끝 ============================== //
// ============================== ls2mod.c 끝 ============================== //
// ============================== ls2mod.c 끝 ============================== //
// ============================== ls2mod.c 끝 ============================== //

#include <pthread.h>

#define nl() { puts(""); }

struct cmdNode {
	char* cmd;
	struct cmdNode* prev;
	struct cmdNode* next;
};

struct cmdNode* cmdHeader; //사용자가 입력한 명령어 모음. cmdHeader 자체는 값이 없고 next에 값이 있음
struct cmdNode* cmdCursor; //현재 선택 중인 cmdNode 위치


void cmdHeaderInit() {
	struct cmdNode* makeCmdNode(char*);

	cmdHeader = makeCmdNode("");
	cmdHeader->prev = NULL;
	cmdHeader->next = NULL;

	cmdCursor = cmdHeader;
}

struct cmdNode* makeCmdNode(char* str) {
	struct cmdNode* temp = (struct cmdNode*)malloc(sizeof(struct cmdNode));
	char* tempstr = (char*)malloc(sizeof(char) * strlen(str));
	strcpy(tempstr, str);
	temp->cmd = tempstr;
	temp->prev = NULL;
	temp->next = NULL;

	return temp;
}

//originalNode 뒤에 nodeToInsert를 연결시킨다
void insertCmdNode(struct cmdNode* originalNode, struct cmdNode* nodeToInsert) {

	nodeToInsert->next = originalNode->next;
	if(nodeToInsert->next != NULL) nodeToInsert->next->prev = nodeToInsert;

	originalNode->next = nodeToInsert;
	nodeToInsert->prev = originalNode;
}

void freeCmdNodes(struct cmdNode* nd) {
	if(nd != NULL) {
		freeCmdNodes(nd->next);

		nd->prev->next = NULL; //앞 노드랑 연결 끊어놓고
		free(nd); //free
	}
}

//cmdHeader 마지막에 node를 새로 붙임
void pushCmdNode(char* str) {
	freeCmdNodes(cmdCursor->next);

	struct cmdNode* temp = makeCmdNode(str);
	insertCmdNode(cmdCursor, temp);
	cmdCursor = temp;
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

//string에 특정 char가 몇 개인지 세는 함수
int countChars(char* str, char c) {
    int counter = 0;
    while(*str != '\0') {
        if(*str == c) {
            counter++;
        }
        str = str + 1;
    }
    return counter;
}

void forkExec2(char** av) {
	if(strcmp(av[0], "cd") == 0) { //cd는 명령어가 없어서 그냥 직접 chdir
		if(strcmp(av[1], "~") == 0) {
			chdir(getenv("HOME"));
		} else {
			chdir(av[1]);
		}

		return;
	}

	int f = fork();

	if(f == 0) { //자식 프로세스
		execvp(av[0], av);
		exit(0);
	} else { //부모 프로세스
		wait(NULL);
	}
}

void forkExec(char* str) {
	int spaceCount = countChars(str, ' ');
	char* av[spaceCount + 1];
	stringTo2darr(str, av);
	forkExec2(av);
}

void execPwd() {
    setTextColor(YELLOW);
    printf("pwd: ");
    fflush(stdout); //안하면 pwd가 또 print됨
	
	char s[] = "pwd";
	forkExec(s);
	setTextColor(DEFAULT);
}

void execLs() {
	char* temp_av[] = { "ls" };
	my_ls(1, temp_av);
}

void showWindow() {
    nl();
    execPwd();
    nl();
    execLs();
    nl();
}

void selectFile(int k) {
	struct file_information* fi = getFileInfoFromHeader(k);

	char* fname = fi->filename;
	int mode = fi->info_p->st_mode;

	if(S_ISDIR(mode)) { //폴더면 폴더 이동
		chdir(fname);
	} else if(mode & (S_IXUSR | S_IXGRP | S_IXOTH)) { //실행 가능한 파일
		char buf[2 + sizeof(fname)] = { "./" };
		strcat(buf, fname);
		forkExec(buf);
	} else { //기타 파일은 vi로 open
		char buf[4 + sizeof(fname)] = { "vi " };
		strcat(buf, fname);
		forkExec(buf);
	}
}

void eraseInput(int i) {
	while(i > 0) {
		printf("\b \b");
		i--;
	}
}

void custom_fgets(char* input) {
	int i = 0;
	char c;

	while((c = getchar()) != '\n') {
		if(c == 127) { //백스페이스
			if(i > 0) {
				printf("\b\b\b   \b\b\b"); //화면에서 문자 지우고
				input[--i] = '\0'; //input에서 지우고
			} else {
				printf("\b\b  \b\b"); //더이상 못 지우는 경우
			}
		} else if(c == '\033') { //방향키
			printf("\b\b\b\b    \b\b\b\b"); //^[[A 같은 거 지우기

			getchar(); //[ 버리고

			switch(getchar()) {
				case 'A': //위
					eraseInput(i);
					i = 0;
					strcpy(input, cmdCursor->cmd);
					printf("%s", input);
					i = strlen(input);
					if(cmdCursor->prev != cmdHeader)
						cmdCursor = cmdCursor->prev;
					continue;
				case 'B': //아래
					eraseInput(i);
					i = 0;
					if(cmdCursor->next != NULL && cmdCursor->next->next != NULL) {
//						if(cmdCursor->prev != cmdHeader) {
//							cmdCursor = cmdCursor->next->next;
//						} else {
//							cmdCursor = cmdCursor->next;
//						}
						cmdCursor = cmdCursor->next->next;
						strcpy(input, cmdCursor->cmd);
						printf("%s", input);
						i = strlen(input);
						cmdCursor = cmdCursor->prev;
					} else if(cmdCursor->next != NULL && cmdCursor->next->next == NULL) {
						cmdCursor = cmdCursor->next;
					}
					continue;
				case 'C': //왼쪽
					break;
				case 'D': //오른쪽
					break;
			}
		} else { //그외 일반 입력
			input[i++] = c;
		}
	}

	input[i++] = '\0';

	pushCmdNode(input);
}

void showTerminal() {
    char input[200];

	showWindow();

	printf("linux explorer: ");

	custom_fgets(input);
//	fgets(input, BUFSIZ, stdin);
//	pushCmdNode(input);

	//input이 1이상의 숫자이면 파일을 선택한 것으로 간주
	int fileNum;
	if((fileNum = atoi(input)) > 0) {
		selectFile(fileNum);
	} else { //명령어를 입력한 경우에는 명령어를 실행
		forkExec(input);
	}
}

int main(int ac, char* av[]) {
	updateHardlinkFile();
	cmdHeaderInit();

    while(1) {
		showTerminal();
    }

	return 0;
}
