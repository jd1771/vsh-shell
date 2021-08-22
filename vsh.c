#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/time.h>


#define TRUE 1
#define FALSE 0
#define MAX_INPUT 80
#define MAX_TOKENS 8
#define MAX_PATH_SIZE 255
#define MAX_DIRS 8


int tok_input(char input[], char *tok_list[]);
int get_dirs(char dir_list[][MAX_PATH_SIZE], char *directory_file);
int search_dirs(char dir_list[][MAX_PATH_SIZE], int dir_count, char* command, char path_buffer[]);
void execute(char *command_list[],char *envp[], int time_check);
void execute_input(char *command_list[], char *envp[], char *input_file, int time_check);
void execute_output(char *command_list[], char *envp[], char *output_file, int time_check);
void execute_input_output(char *command_list[], char *envp[], char *input_file, char *output_file, int time_check);

/* Takes an empty list and fills it with parameters given by the user */
/* Returns the parameter count of the user input */
int tok_input(char input[], char *tok_list[]){
	int token_count = 0;
	char* t = strtok(input," ");
	while (t!=NULL && token_count < MAX_TOKENS){
		tok_list[token_count] = t;
		token_count++;	
		t = strtok(NULL," ");

	}

	return token_count;



}

/* Searches through the vshrc file and appends to the directory list */
/* Returns the directory count */
int get_dirs(char dir_list[][MAX_PATH_SIZE], char *directory_file){
	int dir_count = 0;
	size_t max_path_size = MAX_PATH_SIZE;
	FILE *fp = fopen(directory_file,"r");
	if (fp == NULL){
		return -1;
	}
	while(fgets(dir_list[dir_count],max_path_size,fp)){
		dir_list[dir_count][strlen(dir_list[dir_count])-1] =  '\0';
		dir_count++;
	}

	
	

	
	fclose(fp);
	return dir_count;
}

/* Search the given directories and find the associated binary file with the command entered by the user */
/* Returns a int indicating a success of failure to find the command */
int search_dirs(char dir_list[][MAX_PATH_SIZE], int dir_count, char* command,char path_buffer[]){
	
	for (int i = 0;i < dir_count; i++){
		char path[MAX_PATH_SIZE]  = "";
		strcat(path,dir_list[i]);
		strcat(path,"/");
		strcat(path,command);
		FILE *fp = fopen(path,"r");
		if (fp == NULL){
			continue;
		}
		memcpy(path_buffer,path,sizeof(path));
		return 1;
	


	}
	return -1;	




}

/* Execute a command through a child process given no input and output file */
/* Returns void */
void execute(char *command_list[], char *envp[], int time_check){
	struct timeval before, after;
	int status;
	gettimeofday(&before, NULL);
	pid_t pid = fork();
	if (pid == 0){
		execve(command_list[0],command_list,envp);
	}
	
	waitpid(pid, &status, 0);
	gettimeofday(&after, NULL);
	if (time_check == TRUE){
		fprintf(stdout,"\nCompleted in %lu µs\n",(after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec -before.tv_usec));

	}
	return;



}

/* Execute a command through a child process given an input file */
/* Returns void */
void execute_input(char *command_list[], char *envp[], char *input_file, int time_check){
	int fd;
	int status;
	struct timeval before, after;
	gettimeofday(&before, NULL);
	pid_t pid = fork();
	if (pid == 0){
		fd = open(input_file, O_RDONLY, S_IRUSR);
		if (fd == -1){
			fprintf(stderr,"Invalid input file given\n");
			return;
		}
		dup2(fd,0);
		execve(command_list[0],command_list,envp);
		close(fd);

	}
	
	waitpid(pid, &status, 0);
	gettimeofday(&after, NULL);
	if (time_check == TRUE){
		fprintf(stdout,"\nCompleted in %lu µs\n",(after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec -before.tv_usec));
	}
	return;




}

/* Execute a command through a child process given an output file */
/* Returns void */
void execute_output(char *command_list[], char *envp[], char *output_file,int time_check){
	int fd;
	int status;
	struct timeval before, after;
	gettimeofday(&before, NULL);
	pid_t pid = fork();
	if (pid == 0){
		fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		dup2(fd,1);
		execve(command_list[0],command_list,envp);
		close(fd);
	}
	
	waitpid(pid, &status, 0);
	gettimeofday(&after, NULL);
	if (time_check == TRUE){
	       	fprintf(stdout,"\nCompleted in %lu µs\n",(after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec -before.tv_usec));
	}
	return;



}

/* Execute a command through a child process given an input and output file */
/* Returns void */
void execute_input_output(char *command_list[], char *envp[], char *input_file, char *output_file, int time_check){
	int fd1, fd2;
	int status;
	struct timeval before, after;
	gettimeofday(&before,NULL);
	pid_t pid = fork();
	if (pid == 0){
		fd1 = open(input_file, O_RDONLY, S_IRUSR);
		if (fd1 == -1){
			fprintf(stderr,"Invalid input file given\n");
			return;
		}
		
		fd2 = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		dup2(fd1, 0);
		dup2(fd2, 1);
		execve(command_list[0],command_list,envp);
		close(fd2);
		close(fd1);
	}
	
	waitpid(pid, &status, 0);
	gettimeofday(&after,NULL);
	if (time_check == TRUE){
		fprintf(stdout,"\nCompleted in %lu µs\n",(after.tv_sec - before.tv_sec) * 1000000 + (after.tv_usec -before.tv_usec));
	}
	return;



}

int main(){
	
        while(1){

		char *directory_file = ".vshrc";
	        int time_check = FALSE;
	        int input_check = FALSE;   // If an input file is given in the parameters
	        int output_check = FALSE;  // If an output file is given in the parameters
	        char *input_file;
	        char *output_file;
	        char *command_list[MAX_TOKENS+1]; // List that holds the command entered by the user
	        int command_count = 0;
	        char dir_list[MAX_DIRS][MAX_PATH_SIZE];
	        char path_buffer[MAX_PATH_SIZE];
	        char *envp[] = { 0 };
	        char input[MAX_INPUT];
                char *tok_list[MAX_TOKENS];

		fprintf(stdout,"vsh%% ");
		fflush(stdout);
		fgets(input,MAX_INPUT,stdin);
		

		if (input[strlen(input)-1] == '\n'){       
			input[strlen(input)-1] = '\0';
		}

		if (strcmp(input,"exit") == 0){
			exit(0);
		}
		
		
		int token_count = tok_input(input,tok_list);
	
		for (int i = 0; i<token_count;i++){
			if (output_check == FALSE && strlen(tok_list[i]) >= 2  && tok_list[i][0] == ':' && tok_list[i][1] == ':'){
			        if (strlen(tok_list[i]) ==  2){
		                         fprintf(stderr,"Please enter a file associated with '::'\n");
					 continue;
	                        } 
				output_check = TRUE;
				output_file = tok_list[i] + 2;
				
			}else if(input_check == FALSE && strlen(tok_list[i]) >= 2 && tok_list[i][strlen(tok_list[i])-1] == ':' && tok_list[i][strlen(tok_list[i])-2] == ':'){
				if (strlen(tok_list[i]) ==  2){
					fprintf(stderr,"Please enter a file associated with '::'\n");
					continue;
				}
				input_check = TRUE;
				input_file = tok_list[i];
				input_file[strlen(input_file)-2] = '\0';
				
			}else if (strncmp(tok_list[i],"##",2) == 0){
				time_check = TRUE;
			
			}else{
				command_list[command_count] = tok_list[i];
				command_count++;	
			}
		}

		if (command_count ==  0){				// No point in executing rest of the instructions if the user hasn't entered any commands
			fprintf(stdout,"Please enter a real command\n");
			continue;

		}
		
		int dir_count = get_dirs(dir_list,directory_file);
		if (dir_count == -1){
			fprintf(stdout,".vshrc file not found\n");
			continue;
		}
		
		int command_check = search_dirs(dir_list,dir_count,command_list[0],path_buffer);
		if (command_check == -1){
			fprintf(stdout,"command not found\n");
			continue;
		}
			
		command_list[0] = path_buffer;
		command_list[command_count] = 0;
		command_count++;
		
	        if (input_check == FALSE && output_check == FALSE){
	                execute(command_list,envp, time_check);		
		   		       
	        }else if (input_check == TRUE && output_check == FALSE){
		        execute_input(command_list,envp,input_file, time_check);         	
	 
	        }else if (input_check == FALSE && output_check == TRUE){
			execute_output(command_list,envp,output_file, time_check);
	       
	        }else if (input_check == TRUE && output_check == TRUE){
	                execute_input_output(command_list,envp,input_file,output_file,time_check);
	        }

	       

		
	}
	
	return 0;
}



