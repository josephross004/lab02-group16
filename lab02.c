// ----------------------------------------------
//
#include "shell.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------
// Main function
//
// DO NOT MODIFY
//
int main(int argc, char **argv) {

  int done = false;
  int cmd_cnt = 0;

  char user_input[ CHUNK ] = "\0"; 

  while ( !done ) {

    printf("MyShell(%d)$ ", cmd_cnt++);     // print the prompt
    fgets( user_input, 100, stdin );        // read user input

    if ( parse_input( user_input ) > 0 ) {  // parse the user input
      execute();                            // exeute the command(s)
    }

    unallocate_resources();
  
  }

  return 0;

} // end main function
