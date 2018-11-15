#include <stdio.h>              
#include <sys/wait.h>           
#include <unistd.h>             
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "shell.h" 

void get_cmd_line(char cmd_line[MAX_LINE_SIZE], char arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE], unsigned* arg_num_ptr) {
    unsigned arg_num = 0;                   // number of token extracts by cmd_line
    char buf[MAX_LINE_SIZE];                // buffer to not modified cmd_line
    strcpy(buf, cmd_line);                  // copy cmd_line: strtok do side-effect
    const char* delim = " \n\t";            // delimitators
    char* token = strtok(buf, delim);       // tokeniz buffer
    while (token != NULL) {
        strcpy(arg_buf[arg_num], token);    // copy current token in arg_buf
        token = strtok(NULL, delim);        // extract next token
        arg_num++;
    }
    *arg_num_ptr = arg_num;                 // return number of extracts token to caller
}


void print_cmd_info(void){
	printf("\n");
	printf("			%smkfile (mkf) <file-name> = 'createFile', 		creates empty files\n", COL_B_BLUE);
	printf("			ls <dir-name>  = 'readDir', 				reads the name of all files in a directory\n");
	printf("			open (o) <file-name>  = 'openFile', 			open a file\n");
	printf("			cd <dir-name> = 'changeDir', 				seeks for a directory\n");
	printf("			mkdir (mkd) <dir-name> = 'mkDir', 			creates new directorys in the current one\n");
	printf("			rm <file-name> = 'remove',				removes the file in the current directory\n");
	printf("			rmdir <directory-name> = 'remove',			removes directory and all files contained\n");
	printf("			pwd = 'print working directory'				print path of current directory\n\n");
	printf("			help (h) = 'help', PRINT COMMAND INFO\n");
	printf("			quit (q) = 'quit', QUIT SHELL\n\n%s", COL_GRAY);
}


void print_file_info(void){
	printf("\n");
	printf("			%swrite (w)<data> = 'write', 			write data on file\n", COL_CYAN);
	printf("			read (r)<size>  = 'read', 			reads size bytes on file and move file pointer\n");
	printf("			seek (s)<pos> = 'seek',				move file pointer at pos\n\n");
	printf("			info (i) =  'info',  PRINT FILE INFO\n");
	printf("			help (h) =  'help',  PRINT COMMAND INFO\n");
	printf("			close (c) = 'close', CLOSE FILE\n\n%s", COL_GRAY);
}


int do_file_cmd(FileHandle* f, char arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE], unsigned arg_num){
	
	///CLOSE 
	if (strcmp(arg_buf[0], "close") == 0 || strcmp(arg_buf[0], "c") == 0) return -1;			//run command close = 'close'
	
	///HELP
	else if (strcmp(arg_buf[0], "help") == 0 || strcmp(arg_buf[0], "h") == 0){					//run command help = 'help'
		print_file_info();
	}
	
	///INFO
	else if (strcmp(arg_buf[0], "info") == 0 || strcmp(arg_buf[0], "i") == 0){					//run command info = 'info'
		printf("			       %s%s:\n", COL_CYAN, f->fcb->fcb.name);
		printf("			Position on file = %d\n", f->pos_in_file);
		printf("			Bytes written = %d\n", f->fcb->fcb.written_bytes);
		printf("			Max size to read = %d\n%s", f->fcb->fcb.written_bytes - f->pos_in_file, COL_GRAY);
	}
	
	///WRITE
	else if (strcmp(arg_buf[0], "write") == 0 || strcmp(arg_buf[0], "w") == 0){					//run command w = 'write'
		if(arg_num == 2){	
			int size = strlen(arg_buf[1]);
			char* data = malloc(sizeof(char)*size);
			sprintf(data, "%s", arg_buf[1]);		
			SimpleFS_write(f, data, size);
		}
		else
			printf("%sUsage: write <data>\n%s", COL_RED, COL_GRAY);								//at-least 1 token
	}
	
	///READ
	else if (strcmp(arg_buf[0], "read") == 0 || strcmp(arg_buf[0], "r") == 0){					//run command r = 'read'
		if(arg_num == 2){
			int size = atoi(arg_buf[1]);		
			char* data = malloc(sizeof(char)*size);
			data[size] = '\0';
			SimpleFS_read(f, data, size);
			printf("%s", data);
		}
		else
			printf("%sUsage: read <size>\n%s", COL_RED, COL_GRAY);								//at-least 1 token
	}
	
	///SEEK
	else if (strcmp(arg_buf[0], "seek") == 0 || strcmp(arg_buf[0], "s") == 0){					//run command seek = 'seek'
		if(arg_num > 1){
			int pos = atoi(arg_buf[1]);		
			SimpleFS_seek(f, pos);
		}
		else
			printf("%sUsage: seek <pos>\n%s", COL_RED, COL_GRAY);								//at-least 1 token
	}

	else
        printf("%sSorry command not found, look at commands list with help or h\n%s", COL_RED, COL_GRAY);

	printf("\n");
	return 1;
}


void do_file_loop(FileHandle* f) {
    char     cmd_line[MAX_LINE_SIZE];                    	// buffer for cmd_line
    char     arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE];       	// buffer token cmd_line
    unsigned arg_num;                                    	// number token cmd_line
    
    for (;;) {
        printf("%sSIMPLEFS %s> %s", COL_CYAN, f->fcb->fcb.name, COL_GRAY);     // print string prompt + filename
        if (fgets(cmd_line, MAX_LINE_SIZE, stdin)==NULL) 	// read cmd line
            break;                                       	// quit if NULL
        get_cmd_line(cmd_line, arg_buf, &arg_num);     		// extracts i token
        if (arg_num == 0) continue;                      	// repeat if empty line
        int res = do_file_cmd(f, arg_buf, arg_num);     	// try to run command
        if (res == -1) break;                            	// exit if quit
    }
    return;
}


void do_pwd(DiskDriver* disk, FirstDirectoryBlock* fdb, int block_in_disk){
	if(block_in_disk == -1) return;																//base
	DiskDriver_readBlock(disk, fdb, block_in_disk);												//read current dir 
	block_in_disk = fdb->fcb.directory_block;													
	char dir[MAX_NAME_LEN];
	strncpy(dir, fdb->fcb.name, MAX_NAME_LEN);													//copy name in structure
	do_pwd(disk, fdb, block_in_disk);															//do recursion
	if(strcmp(dir, "/") == 0) printf("%sroot%s/", COL_GREEN, COL_GRAY);							//if / print root
	else printf("%s/", dir);																	//else print dir name 
}

int do_cmd(DirectoryHandle* dh, char arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE], unsigned arg_num){
	
	///QUIT
	if (strcmp(arg_buf[0], "quit") == 0 || strcmp(arg_buf[0], "q") == 0){ 						//run command q = 'quit'
		free(dh);
		return -1;
	}
	
	///HELP
	else if (strcmp(arg_buf[0], "help") == 0 || strcmp(arg_buf[0], "h") == 0){					//run command h = 'help'
		print_cmd_info();
	}
	
	///READ DIRECTORY
	else if (strcmp(arg_buf[0], "ls") == 0){													//run command ls = 'readDir'
		char name[MAX_NAME_LEN];
		char** names = (char**) malloc(sizeof(char*)*dh->dcb->num_entries);
		SimpleFS_readDir(names, dh);
		for(int i = 0; i < dh->dcb->num_entries; i++){
			sprintf(name, "%s", names[i]);
			assert(!strcmp(names[i], name));
			printf("%s  ", names[i]);
		}
		free(names);
	}
	
	///CHANGE DIRECTORY
	else if (strcmp(arg_buf[0], "cd") == 0){													//run command cd = 'changeDir'
		if(arg_num == 2){
			char dirname[MAX_NAME_LEN];
			if(strncmp(arg_buf[1], "..", 2) == 0) 
				SimpleFS_changeDir(dh, arg_buf[1]);
			else{
				sprintf(dirname, "%s%s%s", COL_GREEN, arg_buf[1], COL_GRAY);
				SimpleFS_changeDir(dh, dirname);
			}
		}
		else
			printf("%sUsage: cd <dir-name>\n%s", COL_RED, COL_GRAY);							//at-least 1 token
	}
	
	///CREATE FILE
	else if (strcmp(arg_buf[0], "mkfile") == 0 || strcmp(arg_buf[0], "mkf") == 0){				//run command mkf = 'createFile'
		char name[MAX_NAME_LEN];
		if(arg_num > 1){
			for(int i = 1; i < arg_num; ++i){
				sprintf(name, "%s", arg_buf[i]);
				SimpleFS_createFile(dh, name);
			}
		}
		else
			printf("%sUsage: mkf <file-name>\n%s", COL_RED, COL_GRAY);							//at-least 1 token
	}
	
	///CREATE DIRECTORY
	else if (strcmp(arg_buf[0], "mkdir") == 0 || strcmp(arg_buf[0], "mkd") == 0){				//run command mkdir = 'createDirectory'
		char name[MAX_NAME_LEN];
		if(arg_num > 1){
			for(int i = 1; i < arg_num; ++i){
				sprintf(name, "%s%s%s", COL_GREEN, arg_buf[i], COL_GRAY);
				SimpleFS_mkDir(dh, name);
			}
		}
		else
			printf("%sUsage: mkdir <directory-name>\n%s", COL_RED, COL_GRAY);					//at-least 1 token
	}
	
	///OPEN FILE
	else if (strcmp(arg_buf[0], "open") == 0 || strcmp(arg_buf[0], "o") == 0){					//run command o = 'open'
		if(arg_num == 2){	
			char name[MAX_NAME_LEN];
			sprintf(name, "%s", arg_buf[1]);
			FileHandle* f = SimpleFS_openFile(dh, name);
			if(f == NULL){
				printf("%sINVALID NAME FILE: File doesn't exist\n%s", COL_RED, COL_GRAY);
				return 1;
			}
			do_file_loop(f);
			SimpleFS_close(f);
		}
		else
			printf("%sUsage: open <file-name>\n%s", COL_RED, COL_GRAY);							//at-least 1 token
	}
	
	///REMOVE FILE
	else if (strcmp(arg_buf[0], "rm") == 0){													//run command rm = 'remove file'
		if(arg_num > 1){
			char name[MAX_NAME_LEN];	
			for(int i = 1; i < arg_num; ++i){
				sprintf(name, "%s", arg_buf[i]);
				SimpleFS_remove(dh, name);
			}
		}
		else
			printf("%sUsage: rm <file-name> \n%s", COL_RED, COL_GRAY);							//at-least 2 token
	}
	
	///REMOVE DIRECTORY
	else if (strcmp(arg_buf[0], "rmdir") == 0){													//run command rmdir = 'remove dir'
		if(arg_num > 1){
			char name[MAX_NAME_LEN];
			for(int i = 1; i < arg_num; ++i){
				sprintf(name, "%s%s%s", COL_GREEN, arg_buf[i], COL_GRAY);
				SimpleFS_remove(dh, name);
			}
		}
		else
			printf("%sUsage: rmdir <directory-name> \n%s", COL_RED, COL_GRAY);					//at-least 2 token
	}
	
	///PRINT WORKING DIRECTORY
	else if (strcmp(arg_buf[0], "pwd") == 0){													//run command do_pwd = 'print working directory'
		int this = dh->dcb->fcb.block_in_disk;
		int block_in_disk = dh->dcb->fcb.directory_block;
		do_pwd(dh->sfs->disk, dh->dcb, block_in_disk);
		DiskDriver_readBlock(dh->sfs->disk, dh->dcb, this);
		printf("%s", dh->dcb->fcb.name);
	}
	else
        printf("%sSorry command not found, look at commands list with help or h\n%s", COL_RED, COL_GRAY);

	printf("\n");
	return 1;
}


void do_cmd_loop(DiskDriver* disk, SimpleFS* sfs) {
    char     cmd_line[MAX_LINE_SIZE];                    	// buffer for cmd_line
    char     arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE];       	// buffer token cmd_line
    unsigned arg_num;                                    	// number token cmd_line
    DirectoryHandle* dh = SimpleFS_init(sfs, disk);
	if(dh == NULL){
		SimpleFS_format(sfs);
		dh = SimpleFS_init(sfs, disk);
	}
    for (;;) {
        printf("%s%s%s", COL_B_BLUE, PROMPT, COL_GRAY);     // print string prompt
        if (fgets(cmd_line, MAX_LINE_SIZE, stdin)==NULL) 	// read cmd line
            break;                                       	// quit if NULL
        get_cmd_line(cmd_line, arg_buf, &arg_num);     		// extracts i token
        if (arg_num == 0) continue;                      	// repeat if empty line
        int res = do_cmd(dh, arg_buf, arg_num);     		// try to run command
        if (res == -1) break;                            	// exit if quit
    }
}


// ---------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------
int main(int args, char** argv) {
	DiskDriver disk;
	SimpleFS sfs;
	
	DiskDriver_init(&disk, "disk_file.txt", 2048);																	//Init disk
	
	printf("\n%s***************************************************************************"
				"***************************************************************************\n", COL_B_BLUE);
	printf("								Welcome to SIMPLEFS\n");
	printf("*******************************************************************************"
					"***********************************************************************%s\n", COL_GRAY);
	print_cmd_info();																								
    do_cmd_loop(&disk, &sfs);																						//start loop shell
    printf("\n								    %sGOOD LUCK\n\n\n%s", COL_YELLOW, COL_GRAY);
    return 0;
}
