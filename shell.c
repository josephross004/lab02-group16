// I pledge the COMP530 honor code.

// ----------------------------------------------
// These are the only libraries that can be
// used. Under no circumstances can additional
// libraries be included
#include "shell.h"
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

input_struct* shell = NULL;

// ----------------------------------------
// Functions that CANNOT be modified
// ----------------------------------------
// ------------------------------------
// Removes leading and trailing whitespace
// from a string.
//
// This function is useful when parsing
// and cleaning user input.
void trim(char* str) {

    char* p = str;       
    char* start = str;    /* first non-ws */
    char* last = NULL;    /* last non-ws */
    size_t len = 0;

    /* find first non-whitespace */
    while (*p && isspace((unsigned char)*p))
        p++;
    start = p;

    /* find last non-whitespace */
    while (*p) {
        if (!isspace((unsigned char)*p))
            last = p;
        p++;
    }

    if (last) {
        len = (size_t)(last - start + 1);
        /* remove white-space */
        if (start != str)
            memmove(str, start, len);

        str[len] = '\0';
    } else {
        /* string was empty or all whitespace */
        str[0] = '\0';
    }

} // end trim function


// ------------------------------------
// Executes a built-in command.
//
// This function handles built-in commands only,
// specifically `cd` and `exit`. These commands
// are executed directly by the shell process
// rather than by creating a child process.
//
// See the "Shell" section in the README.
void builtin( cmd_t* cmd ) {

  if ( strcmp(cmd->argv[0], "exit") == 0) {
    unallocate_resources();
    exit(0);
  } else if ( strcmp( cmd->argv[0], "cd") == 0 ) {
    if (cmd->argc == 1)
      chdir(getenv("HOME"));  // cd with no arguments
    else if ( cmd->argc == 2)
      chdir(cmd->argv[1]);  // cd with 1 arg
    else
      fprintf(stderr, "cd: Too many arguments\n");
  }

} // end builtin() function

// ------------------------------------
// Determines whether a command is a built-in.
//
// This function checks whether the command
// specified by cmd_t->argv[0] is a built-in
// command. Only `cd` and `exit` are treated
// as built-ins.
//
// Returns:
//   true  if cmd_t->argv[0] is "cd" or "exit"
//   false otherwise
bool is_builtin(cmd_t* cmd) {
    
    if (strcmp(cmd->argv[0], "cd") == 0 || strcmp(cmd->argv[0], "exit") == 0)
        return true;
    return false;

} // end is_builtin() function


// ----------------------------------------
// Functions that CAN be modified
// ----------------------------------------

// --------------------------------------
// Executes the parsed user input after the
// input_struct and associated cmd_t structures
// have been constructed by parse_input() and
// parse_command().
//
// This function:
//   - Sets up pipes if needed.
//   - Executes built-in commands directly in
//     the shell process.
//   - Otherwise, creates one child process per
//     command using fork().
//   - In each child, configures any required
//     redirection and pipe endpoints, closes
//     unused file descriptors, then calls
//     execvp() to execute the program.
//   - In the parent, closes unused file
//     descriptors and waits for all child
//     processes to complete.
//
// See the "Shell" section in the README for
// additional details.
void execute() {

  // TODO: your solution






} // end execute() function

// --------------------------------------
// Called by parse_input() to further process
// an individual command string.
//
// This function parses the command string and
// constructs a cmd_t structure by identifying
// and setting the following fields:
//   - argv
//   - argc
//   - in_file
//   - out_file
//   - append
//   - stderr (honor section only)
//
// After the cmd_t structure is initialized,
// it is appended to the end of the linked list
// of commands.
//
// See the "Shell → Internal Representation"
// section in the README for examples.
void parse_command( char* command ) {

  // TODO: your solution




  





} // end parse_command function

// --------------------------------------
// Identifies and separates each command in the
// user_input string and builds the input_struct.
//
// If the input is not empty and does not contain
// a pipe symbol (`|`), then there is only one
// command. If one or more pipes are present,
// the input is split into multiple command
// strings (the number of commands equals the
// number of pipes plus one).
//
// For each identified command string, this
// function calls parse_command() to further
// process the command.
//
// Returns: The total number of commands (i.e., 
// shell_struct->total_cmd_t).
//
// See the "Shell → Internal Representation"
// section in the README for examples.
int parse_input( char* user_input ) {

  // TODO: your solution




  
  return 0; // this is just a place holder
  
} // end parse_input() function

// --------------------------------------
// Frees all dynamically allocated memory associated
// with the input_struct and its linked cmd_t structures.
//
// This includes any heap-allocated fields such as:
// - argv
// - in_file
// - out_file
// - each cmd_t node in the linked list
// - user_input
// - the input_struct itself
//
// See the "Shell → Internal Representation"
// section in the README for additional details.
void unallocate_resources() {

  // TODO: your solution






} // end unallocate_resources function


// --------------------------------------
// This function is provided for debugging
// purposes.
//
// Example code is included, but you are
// encouraged to modify it as needed to help
// you and your team debug and inspect the
// internal representation of the parsed
// user input.
void debug() {

  if ( shell != NULL ) {
    printf(" --------- DEBUG --------------\n");
    printf("User input (%s)\n", shell->user_input );
    printf("Total # of commands (%d)\n", shell->total_cmd_t );

    cmd_t* node = shell->head_node;
    while ( node != NULL ) {
      for ( int i=0; i<node->argc; i++ ) {
        printf( "argv[%d]=%s\n", i, node->argv[i] );
      }
      if ( node->in_file != NULL ) printf("IN_FILE (%s)\n", node->in_file );
      if ( node->out_file != NULL ) printf("OUT_FILE (%s)\n", node->out_file );
      if ( node->append ) printf("OUT FILE APPEND\n");
      if ( node->stderr ) printf("OUT FILE STDERR\n");
      node = node->next_node;
    }

  }

} // end debug function

