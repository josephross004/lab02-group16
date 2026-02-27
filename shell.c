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

  if (shell == NULL || shell->head_node == NULL) return;

  // If there is only one command and it is a builtin, we will run it directly
  if (shell->total_cmd_t == 1 && is_builtin(shell->head_node)) {
    builtin(shell->head_node);
    return;
  }

  int num_cmds = (int)shell->total_cmd_t;
  int prev_read_fd = -1;

  cmd_t* cmd = shell->head_node;
  pid_t* pids = (pid_t*)malloc(sizeof(pid_t) * num_cmds);
  if (pids == NULL) return;

  int i = 0;

  while (cmd != NULL) {
    int pipefd[2];

    // Creating a pipe if there is a next command in the pipeline
    if (cmd->next_node != NULL) {
      if (pipe(pipefd) == -1) {
        perror("pipe");
        free(pids);
        return;
      }
    }

    pids[i] = fork();

    if (pids[i] == -1) {
      perror("fork");
      free(pids);
      return;
    }

    if (pids[i] == 0) {
      // --- Child process ---

      // If not the first command, read input from previous pipe
      if (prev_read_fd != -1) {
        dup2(prev_read_fd, STDIN_FILENO);
        close(prev_read_fd);
      }

      // If not the last command, send output to the next pipe
      if (cmd->next_node != NULL) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
      }

      // Handling input redirection: < filename
      if (cmd->in_file != NULL) {
        int fd = open(cmd->in_file, O_RDONLY);
        if (fd == -1) {
          perror("open");
          exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
      }

      // Handling output redirection: > or >> filename
      if (cmd->out_file != NULL) {
        int fd;
        if (cmd->append) {
          fd = open(cmd->out_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
        } else {
          fd = open(cmd->out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }
        if (fd == -1) {
          perror("open");
          exit(1);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      }

      // Executing the command
      execvp(cmd->argv[0], cmd->argv);
      perror("execvp");
      exit(1);
    }

    // --- Parent process ---

    // Closing the previous pipe read end (no longer needed by parent)
    if (prev_read_fd != -1) {
      close(prev_read_fd);
    }

    // Saving the read end of the current pipe for the next child
    if (cmd->next_node != NULL) {
      close(pipefd[1]);
      prev_read_fd = pipefd[0];
    }

    cmd = cmd->next_node;
    i++;
  }

  // Waiting for all child processes to finish
  for (int j = 0; j < num_cmds; j++) {
    waitpid(pids[j], NULL, 0);
  }

  free(pids);

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

  if (command == NULL) return;

  trim(command);
  if (command[0] == '\0') return;

  cmd_t* node = (cmd_t*)calloc(1, sizeof(cmd_t));
  if (node == NULL) return;

  node->argv = NULL;
  node->argc = 0;
  node->in_file = NULL;
  node->out_file = NULL;
  node->append = false;
  node->stderr = false;
  node->next_node = NULL;

  int cap = 4;
  char** args = (char**)calloc((size_t)cap, sizeof(char*));
  if (args == NULL) {
    free(node);
    return;
  }

  char* p = command;

    
  while (*p) {

    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '\0') break;

    if (*p == '<' || *p == '>') {

      bool is_in = (*p == '<');
      bool is_out = (*p == '>');
      bool is_append = false;

      if (is_out) {
        if (*(p + 1) == '>') {
          is_append = true;
          p += 2;
        } else {
          p += 1;
        }
      } else {
        // <
        p += 1;
      }

      while (*p && isspace((unsigned char)*p)) p++;

      char* start = p;
      while (*p && !isspace((unsigned char)*p)) p++;
      size_t len = (size_t)(p - start);

      if (len > 0) {
        char* fname = (char*)malloc(len + 1);
        if (fname != NULL) {
          memcpy(fname, start, len);
          fname[len] = '\0';

          if (is_in) {
            if (node->in_file) free(node->in_file);
            node->in_file = fname;
          } else if (is_out) {
            if (node->out_file) free(node->out_file);
            node->out_file = fname;
            node->append = is_append;
          } else {
            free(fname);
          }
        }
      }

      continue;
    }

    char* token = NULL;
    if (*p == '\"') {
      char buffer[1000];
      int bi = 0;
      p++; 
      
      while (*p && *p != '\"') {
        if (bi < 999) { 
            buffer[bi++] = *p;
        }
        p++;
      }
      buffer[bi] = '\0'; 

      if (*p == '\"') p++; 


      char* quoted_tok = (char*)malloc(bi + 1);
      if (quoted_tok != NULL) {
        memcpy(quoted_tok, buffer, bi + 1);

        
        if (node->argc >= cap) {
          int newcap = cap * 2;
          char** tmp = (char**)realloc(args, newcap*sizeof(char*));
          if (tmp !=NULL) {
            args = tmp;
            cap = newcap;
          }
        }
        args[node->argc] = quoted_tok;
        node->argc += 1;
      }
      
      continue; 
    }

    if (token != NULL) {
        if (node->argc >= cap) {
            cap *= 2;
            char** tmp = (char**)realloc(args, (size_t)cap * sizeof(char*));
            if (tmp == NULL) { free(token); break; }
            args = tmp;
        }
        args[node->argc++] = token;
    }

    char* start = p;
    
    while (*p && !isspace((unsigned char)*p) && *p != '<' && *p != '>') p++;
    size_t len = (size_t)(p - start);
    if (len == 0) continue;

    char* tok = (char*)malloc(len + 1);
    if (tok == NULL) continue;
    memcpy(tok, start, len);
    tok[len] = '\0';

    if (node->argc >= cap) {
      int newcap = cap * 2;
      char** tmp = (char**)realloc(args, (size_t)newcap * sizeof(char*));
      if (tmp == NULL) {
        free(tok);
        break;
      }
      for (int i = cap; i < newcap; i++) tmp[i] = NULL;
      args = tmp;
      cap = newcap;
    }

    args[node->argc] = tok;
    node->argc += 1;
  }

  if (node->argc == 0) {
    if (node->in_file) free(node->in_file);
    if (node->out_file) free(node->out_file);
    free(args);
    free(node);
    return;
  }

  node->argv = (char**)calloc((size_t)node->argc + 1, sizeof(char*));
  if (node->argv == NULL) {
    for (int i = 0; i < node->argc; i++) free(args[i]);
    if (node->in_file) free(node->in_file);
    if (node->out_file) free(node->out_file);
    free(args);
    free(node);
    return;
  }

  for (int i = 0; i < node->argc; i++) node->argv[i] = args[i];
  node->argv[node->argc] = NULL;
  free(args);

  if (shell->head_node == NULL) {
    shell->head_node = node;
  } else {
    cmd_t* cur = shell->head_node;
    while (cur->next_node != NULL) cur = cur->next_node;
    cur->next_node = node;
  }
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

  if (user_input == NULL) return 0;

  trim(user_input);

  if (user_input[0] == '\0') {
    shell = (input_struct*)calloc(1, sizeof(input_struct));
    if (shell == NULL) return 0;
    shell->user_input = (char*)calloc(1, sizeof(char));
    shell->total_cmd_t = 0;
    shell->head_node = NULL;
    return 0;
  }

  shell = (input_struct*)calloc(1, sizeof(input_struct));
  if (shell == NULL) return 0;

  size_t ulen = strlen(user_input);
  shell->user_input = (char*)malloc(ulen + 1);
  if (shell->user_input == NULL) {
    free(shell);
    shell = NULL;
    return 0;
  }
  memcpy(shell->user_input, user_input, ulen + 1);

  shell->total_cmd_t = 0;
  shell->head_node = NULL;

  char* s = shell->user_input;
  char* start = s;
  char* p = s;

  while (true) {
    if (*p == '\"') {
      quote = !quote;
    }


    if ((*p == '|'  && !quote )|| *p == '\0') {
      size_t len = (size_t)(p - start);

      char* segment = (char*)malloc(len + 1);
      if (segment != NULL) {
        memcpy(segment, start, len);
        segment[len] = '\0';
        trim(segment);

        if (segment[0] != '\0') {
          parse_command(segment);
          shell->total_cmd_t += 1;
        }
        free(segment);
      }

      if (*p == '\0') break;
      p++;
      start = p;
      continue;
    }
    p++;
  }

  unsigned int count = 0;
  cmd_t* cur = shell->head_node;
  while (cur != NULL) {
    count++;
    cur = cur->next_node;
  }
  shell->total_cmd_t = count;

  return (int)shell->total_cmd_t;
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
  if (shell == NULL) {
    return;
  }

  cmd_t* current = shell->head_node;
  while (current != NULL) {
    
    cmd_t* upcoming = current->next_node;
    for (int i=0; i<current->argc; i++){
      free(current->argv[i]);
    }
    free(current->argv);

    if (current->in_file){
      free(current->in_file);
    }

    if (current->out_file){
      free(current->out_file);
    }

    free(current);
    current=upcoming;

  }
  
  if (shell->user_input) {
    free(shell->user_input);
  }

  free(shell);
  //final reset
  shell = NULL;

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

