#pragma once
#include "simplefs.h"
#include "disk_driver.h"


#define MAX_LINE_SIZE 1024     			// max len cmd_line
#define MAX_NUM_ARGS  64        		// max number of token on cmd_line
#define PROMPT  "SIMPLEFS> "   
#define BLOCK_SIZE 512

//macro colors
#define COL(x) "\033[" #x ";1m"			//to add in front of a string
#define COL_GREEN COL(32)
#define COL_RED COL(31)
#define COL_YELLOW COL(33)
#define COL_BLUE COL(34)
#define COL_B_BLUE COL(94)
#define COL_CYAN COL(36)
#define COL_WHITE COL(37)
#define COL_GRAY "\033[0m"				//to reset shell color

// ---------------------------------------------------------------------
//  get_cmd_line
// ---------------------------------------------------------------------
// extracts args by cmd_line
// parameters:
//   - cmd_line: [input] stringa di input contenente la riga di comando
//   - arg_buf: [output] array in cui scrivere i token della riga di comando
//   - arg_num_ptr: [output] puntatore a buffer in cui scrivere numero di token in tok_buf
void get_cmd_line(char cmd_line[MAX_LINE_SIZE], char arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE], unsigned* arg_num_ptr);



// ---------------------------------------------------------------------
//  print_cmd_info
// ---------------------------------------------------------------------
// print info about SimpleFS
void print_cmd_info(void);

// ---------------------------------------------------------------------
//  print_file_info
// ---------------------------------------------------------------------
// print info about file commands
void print_file_info(void);


// ---------------------------------------------------------------------
// do_pwd
// ---------------------------------------------------------------------
// recursive function for print working directory
void do_pwd(DiskDriver* disk, FirstDirectoryBlock* fdb, int block_in_disk);


// ---------------------------------------------------------------------
//  do_cmd
// ---------------------------------------------------------------------
// try to run a command
// parameters:
//	 - dh: handle to current directory
//   - tok_buf: [input] array of tokens from cmd_line
//   - tok_num: [input] number of tokens in tok_buf
// return:
//    -1 if quit
//     1 if is a valid command
int do_cmd(DirectoryHandle* dh, char arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE], unsigned arg_num);


// ---------------------------------------------------------------------
//  do_file_cmd
// ---------------------------------------------------------------------
// try to run a file command
// parameters:
//   - f: handle to current file
//   - tok_buf: [input] array of tokens from cmd_line
//   - tok_num: [input] number of tokens in tok_buf
// return:
//    -1 if close file
//     1 if is a valid command
int do_file_cmd(FileHandle* f, char arg_buf[MAX_NUM_ARGS][MAX_LINE_SIZE], unsigned arg_num);

// ---------------------------------------------------------------------
//  do_cmd_loop
// ---------------------------------------------------------------------
// run loop of read/run command
void do_cmd_loop(DiskDriver* disk, SimpleFS* sfs);

// ---------------------------------------------------------------------
//  do_file_loop
// ---------------------------------------------------------------------
// run loop of file command
void do_file_loop(FileHandle* f);
