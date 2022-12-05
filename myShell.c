// @Title  Myshell
// @Description  Linux Shell 的简单实现
// @Author  MGAronya（张健）
// @Update  MGAronya（张健）  2022-12-5 0:33
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<pwd.h>
#include<wait.h>
#include<sys/types.h>
#include <readline/readline.h>
#include <readline/history.h>


// 此处用于定义颜色
// L_GREEN 绿色
#define L_GREEN "\e[1;32m"
// L_BLUE 蓝色
#define L_BLUE  "\e[1;34m"
// L_RED 红色
#define L_RED   "\e[1;31m"
// WHITE 白色
#define WHITE   "\e[0m"

// 定义基础变量
#define TRUE 1
#define FALSE 0

// lastdir		存储当前目录
char lastdir[100];

// command		存储用户输入的指令
char command[BUFSIZ];

// argv 存储命令与命令参数信息等
char argv[100][100];

// argvtmp1 指令1
char **argvtmp1;

// argvtmp2 指令2
char **argvtmp2;

// argv_redirect 重定向的位置
char argv_redirect[100];

// argc 命令词数
int  argc;

// BUILTIN_COMMAND 是否是内建指令
int BUILTIN_COMMAND = 0;

// PIPE_COMMAND 是否是管道指令
int PIPE_COMMAND = 0;

// REDIRECT_COMMAND 是否是重定向指令
int REDIRECT_COMMAND = 0;

// set_prompt 设置提示用户输入
void set_prompt(char *prompt);

// analysis_command 分析用户输入的指令
int analysis_command();

// builtin_command 内建指令处理函数
void builtin_command();

// do_command 其它指令的处理函数
void do_command();

// help help指令
void help();

void initial();

// init_lastdir 初始化所在目录
void init_lastdir();

void history_setup();
void history_finish();
void display_history_list();

// @title    main
// @description   程序入口
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   int
int main(){
	// prompt 存储提示用户输入的信息
	char prompt[BUFSIZ];
	// line 存储用户输入的信息
	char *line;
	
	// TODO 初始化，将当前目录复制到lastdir
	init_lastdir();

	// TODO 历史记录的初始配置
	history_setup();	
	// TODO 死循环获取用户输入
	while(1) {
		// TODO 打印出提示用户输入的信息
		set_prompt(prompt);

		// TODO 用户的输入为空则退出
		if(!(line = readline(prompt))) 
			break;
		// TODO 用户输入信息，记录于历史记录中
		if(*line)
			add_history(line);
		
		// TODO 将用户输入信息存储在用户输入指令中
		strcpy(command, line);
		// TODO 尝试解析用户输入的指令
		if(!(analysis_command()))
			// TODO 如果是内建指令
			if(BUILTIN_COMMAND)
				builtin_command();		
			// TODO 否则使用指令执行函数
			else
				do_command();
		// TODO 执行完后重新初始化
		initial();
	}
	// TODO 历史记录存入
	history_finish();
	
	return 0;
}

// @title    set_prompt
// @description   打印出等待用户输入的信息
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    char *prompt	输入提数用户输入的原始信息
// @return   void
void set_prompt(char *prompt){
	// hostname 用于存储主机名
	char hostname[100];
	// cwd 用于存储当前运行目录
	char cwd[100];
	// super 超级用户
	char super = '#';
	// TODO 用 "/" 分割当前进程运行目录
	char delims[] = "/";	

	// passwd 用户相关信息
	struct passwd* pwp;
	
	// TODO 尝试读出主机名
	if(gethostname(hostname,sizeof(hostname)) == -1)
		// TODO 查看主机名失败，则使用unknow
		strcpy(hostname,"unknown");
	// TODO 获取用户id，并尝试取出用户相关信息 
	pwp = getpwuid(getuid());	
	// TODO 尝试取出当前目录
	if(!(getcwd(cwd,sizeof(cwd))))
		// TODO 查看当前目录失败，则使用unknow
		strcpy(cwd,"unknown");	

	char cwdcopy[100];
	strcpy(cwdcopy,cwd);
	// TODO 使用分隔符分割目录
	char *first = strtok(cwdcopy,delims);
	// TODO 空分割尝试取出目录所属用户
	char *second = strtok(NULL,delims);
	// TODO 当前目录在home，且为所属用户，将其简化为"~"
	if(!(strcmp(first,"home")) && !(strcmp(second,pwp->pw_name))){
		int offset = strlen(first) + strlen(second)+2;
		char newcwd[100];
		// p 原来的目录
		char *p = cwd;
		// q 修改后的目录
		char *q = newcwd;
		// TODO 偏移至~之后的目录信息
		p += offset;
		// TODO 逐一复制
		while(*(q++) = *(p++));
		char tmp[100];
		// TODO 生成新的目录字符串
		strcpy(tmp,"~");
		strcat(tmp,newcwd);
		strcpy(cwd,tmp);			
	}	
	
	// TODO 如果是超级用户
	if(getuid() == 0)
		super = '#';
	// TODO 否则
	else
		super = '$';
	// 打印出提示用户输入信息
	sprintf(prompt, "\001\e[1;32m\002%s@%s\001\e[0m\002:\001\e[1;31m\002%s\001\e[0m\002%c",pwp->pw_name,hostname,cwd,super);	
	
}


// @title    analysis_command
// @description   分析用户输入的指令
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   void
int analysis_command(){    
	int i = 1;
	char *p;
	// delims 	用空格分割命令
	char delims[] = " ";

	// argc 命令长度
	argc = 1;
	
	// TODO 使用空格分割命令，并写入argv[0]
	strcpy(argv[0],strtok(command,delims));

	// TODO 读出命令信息并写入到avgv中
	while(p = strtok(NULL,delims)){
		strcpy(argv[i++],p);
		argc++;
	}
	
	// TODO 如果命令是 exit help cd 中的其中一个，将BUILTIN_COMMAND设为1
	if(!(strcmp(argv[0],"exit"))||!(strcmp(argv[0],"help"))|| !(strcmp(argv[0],"cd"))){
		BUILTIN_COMMAND = 1;	
	}
	int j;
	// TODO 是否是管道命令
	int pipe_location;
	for(j = 0;j < argc;j++){
		// TODO 记录管道过滤下标
		if(strcmp(argv[j],"|") == 0){
			PIPE_COMMAND = 1;
			pipe_location = j;				
			break;
		}	
	}
	
	// TODO 是否是重定向指令
	int redirect_location;
	for(j = 0;j < argc;j++){
		// TODO 记录重定向下标
		if(strcmp(argv[j],">") == 0){
			REDIRECT_COMMAND = 1;
			redirect_location = j;				
			break;
		}
	}

	// TODO 如果是管道指令
	if(PIPE_COMMAND){
		// argvtmp1 管道前的指令
		argvtmp1 = malloc(sizeof(char *)*pipe_location + 1);
		int i;	
		// TODO 获取管道前的指令
		for(i = 0;i < pipe_location + 1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i <= pipe_location)
				strcpy(argvtmp1[i],argv[i]);	
		}
		argvtmp1[pipe_location] = NULL;
		
		//argvtmp2 管道后的指令
		argvtmp2 = malloc(sizeof(char *)*(argc - pipe_location));
		int j;	
		// TODO 获取管道后的指令
		for(j = 0;j < argc - pipe_location;j++){
			argvtmp2[j] = malloc(sizeof(char)*100);
			if(j <= pipe_location)
				strcpy(argvtmp2[j],argv[pipe_location + 1 + j]);	
		}
		argvtmp2[argc - pipe_location - 1] = NULL;
		
	}
	// TODO 如果是重定向指令
	else if(REDIRECT_COMMAND){
		// TODO 把定向的位置放入argv_redirect
		strcpy(argv_redirect,argv[redirect_location + 1]);
		// argvtmp1 重定向前的指令
		argvtmp1 = malloc(sizeof(char *)*redirect_location + 1);
		int i;	
		// TODO 获取重定向前的指令
		for(i = 0;i < redirect_location + 1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i < redirect_location)
				strcpy(argvtmp1[i],argv[i]);	
		}
		argvtmp1[redirect_location] = NULL;
	}
	// TODO 如果是普通指令
	else{
		// argvtmp1 读入普通指令
		argvtmp1 = malloc(sizeof(char *)*argc+1);
		int i;	
		// TODO 读入普通指令
		for(i = 0;i < argc + 1;i++){
			argvtmp1[i] = malloc(sizeof(char)*100);
			if(i < argc)
				strcpy(argvtmp1[i],argv[i]);	
		}
		argvtmp1[argc] = NULL;
	}

	return 0;
}


// @title    builtin_command
// @description   内建指令的处理函数
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   void
void builtin_command(){
	// passwd 用于存储用户信息
	struct passwd* pwp;
	// TODO 如果是退出指令
	if(strcmp(argv[0],"exit") == 0)
		// TODO 直接退出
		exit(EXIT_SUCCESS);
	// TODO 如果是帮助指令
	else if(strcmp(argv[0],"help") == 0)
		// TODO 执行帮助函数
		help();
	// TODO 如果是切换目录指令
	else if(strcmp(argv[0],"cd") == 0){
		// cd_path 存储切换路径
		char cd_path[100];
		// TODO 如果切换指令长度为0
		if((strlen(argv[1])) == 0 ){
			// TODO 获取用户信息
			pwp = getpwuid(getuid());
			// TODO 获取用户的home目录
			sprintf(cd_path,"/home/%s",pwp->pw_name);
			// TODO 将用户的home目录存入cd_path
			strcpy(argv[1],cd_path);
			// TODO 指令词数增加
			argc++;			
		}
		// TODO 如果读出~
		else if((strcmp(argv[1],"~") == 0) ){
			// TODO 获取用户信息
			pwp = getpwuid(getuid());
			// TODO 获取用户的home目录
			sprintf(cd_path,"/home/%s",pwp->pw_name);
			// TODO 将用户的home目录存入cd_path
			strcpy(argv[1],cd_path);			
		}
	
		// TODO 如果切换至指定目录失败
		if((chdir(argv[1]))< 0){
			printf("cd failed in builtin_command()\n");
		}
	}
}

// @title    do_command
// @description   其它指令的处理函数
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   void
void do_command(){

	// TODO 如果是管道指令
	if(PIPE_COMMAND){
		int fd[2],res;
		int status;
		
		// TODO 尝试创建管道
		res = pipe(fd);
	
		// TODO 管道创建失败
		if(res == -1)
			printf("pipe failed in do_command()\n");
		// TODO 尝试生成子进程
		pid_t pid1 = fork();
		// TODO 子进程生成失败
		if(pid1 == -1)
			printf("fork failed in do_command()\n");		
		// TODO 如果是子进程
		else if(pid1 == 0){
				// TODO 打开输出
				dup2(fd[1],1);
				// TODO 关闭读入
				close(fd[0]);
				// TODO 尝试执行命令1
				if(execvp(argvtmp1[0],argvtmp1) < 0)
					printf("%s:command not found\n",argvtmp1[0]);				
		}// TODO 如果是父进程
		else{
			//TODO 等待子进程
			waitpid(pid1,&status,0);
			// TODO 获取新的子进程
			pid_t pid2 = fork();
			// TODO 子进程获取失败
			if(pid2 == -1)
				printf("fork failed in do_command()\n");
			// TODO 为新创建的子进程	
			else if(pid2 == 0){
				// TODO 关闭写入
				close(fd[1]);
				// TODO 打开输出
				dup2(fd[0],0);
				// TODO 尝试执行命令2
				if(execvp(argvtmp2[0],argvtmp2) < 0)
					printf("%s:command not found\n",argvtmp2[0]);		
			}
			// TODO 父进程
			else{
				// TODO 关闭读入读出
				close(fd[0]);
				close(fd[1]);
				// TODO 等待子进程
				waitpid(pid2,&status,0);
			}
		}
	}
	// TODO 如果是重定向指令
	else if(REDIRECT_COMMAND){
		// TODO 创建子进程
		pid_t pid = fork();	
		// TODO 子进程创建失败
		if(pid == -1)
			printf("fork failed in do_command()\n");	
		// TODO 如果是子进程	
		else if(pid == 0){
			int redirect_flag = 0;
			FILE* fstream;
			// TODO 打开重定向到的文件位置
			fstream = fopen(argv_redirect,"w+");
			freopen(argv_redirect,"w",stdout);
			// TODO 尝试执行指令
			if(execvp(argvtmp1[0],argvtmp1) < 0)
				// TODO 指令执行失败
				redirect_flag = 1;
			// TODO 关闭输出		
			fclose(stdout);
			// TODO 关闭文件流
			fclose(fstream);
			// TODO 如果命令执行失败
			if(redirect_flag)
				printf("%s:command not found\n",argvtmp1[0]);	
		}
		// 如果是父进程
		else
			// TODO 等待子进程
			wait(NULL);	 
	}
	// TODO 普通指令
	else{
		// TODO 创建子进程
		pid_t pid = fork();	
		// TODO 子进程创建失败
		if(pid == -1)
			printf("fork failed in do_command()\n");	
		// TODO 如果是子进程	
		else if(pid == 0)
			// TODO 尝试执行命令
			if(execvp(argvtmp1[0],argvtmp1) < 0)
				printf("%s:command not found\n",argvtmp1[0]);
		// TODO 如果是父进程			
		else
			// TODO 等待子进程
			wait(NULL);	
	}

    // TODO 释放空间
    free(argvtmp1);
	free(argvtmp2);
}

// @title    help
// @description   help指令的处理函数
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   void
void help(){
		printf(
"  __  __    _____ \n"                                                  
" |  \\/  |  / ____|     /\\                   \n"                        
" | \\  / | | |  __     /  \\     _ __    ___    _ __    _   _    __ _ \n"
" | |\\/| | | | |_ |   / /\\ \\   | '__|  / _ \\  | '_ \\  | | | |  / _` |\n"
" | |  | | | |__| |  / ____ \\  | |    | (_) | | | | | | |_| | | (_| |\n"
" |_|  |_|  \\_____| /_/    \\_\\ |_|     \\___/  |_| |_|  \\__, |  \\__,_|\n"
"                                                       __/ |        \n"
"                                                      |___/         \n\n\n\n\n");
//打开文件
	FILE *pfile = fopen("./README.md", "rb");
	if (pfile == NULL) {
		printf("open file failed!\n");
		exit(0);
	}

	//读取文件长度,读完后把文件指针复位到开头
	fseek(pfile,0, SEEK_END);
	int file_length = ftell(pfile);
	rewind(pfile);

	//申请一段内存空间用来放文件中的文本内容，并置零
	int text_length = file_length * sizeof(char) + 1;
	char* text_buffer = (char*)malloc(text_length);
	memset(text_buffer, 0, text_length);

	//把文件文本内容读到申请的内存空间中
	int read_length = fread(text_buffer, 1, file_length, pfile);
	if (read_length != file_length) {
		printf("read file failed!\n");
		exit(0);
	}

	//打印文件文本内容，释放申请的内存空间，关闭文件流
	printf(text_buffer);
	free(text_buffer);
	fclose(pfile);

}


// @title    initial
// @description   初始化各种参数
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   void
void initial(){
	int i = 0;	
	for(i = 0;i < argc;i++){
		strcpy(argv[i],"\0");	
	}
	argc = 0;
	BUILTIN_COMMAND = 0;
	PIPE_COMMAND = 0;
	REDIRECT_COMMAND = 0;
}


// @title    init_lastdir
// @description   将当前工作目录的绝对路径复制到参数lastdir所指的内存空间中，以此初始化lastdir
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   int
void init_lastdir(){
	// TODO 获取当前目录
	getcwd(lastdir, sizeof(lastdir));
}

// @title    history_setup
// @description   设置历史记录
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   void
void history_setup(){
	// 使用历史记录
	using_history();
	stifle_history(50);
	read_history("/tmp/msh_history");	
}

// @title    history_finish
// @description   添加历史记录
// @auth      MGAronya（张健）       2022-12-5 12:15
// @param    void
// @return   void
void history_finish(){
	// TODO 将历史记录添加至 /tmp/msh_history 文件中
	append_history(history_length, "/tmp/msh_history");
	history_truncate_file("/tmp/msh_history", history_max_entries);
}


void display_history_list(){
	HIST_ENTRY** h = history_list();
	if(h) {
		int i = 0;
		while(h[i]) {
			printf("%d: %s\n", i, h[i]->line);
			i++;
		}
	}
}

