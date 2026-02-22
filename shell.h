// -----------------------------------
// 	COMP 530: Operating Systems
//
//	Spring 2026 - Lab 2
// -----------------------------------

#include <stdbool.h>

// ----------------------------------------
// Global variables that CANNOT be modified
// ----------------------------------------
#define PASS 0
#define FAIL 1
#define CHUNK 256


// ----------------------------------------
// Global variables that CAN be modified
// ----------------------------------------
#define DEBUG 0 


// ----------------------------------------
// Structs that CANNOT be modified
// ----------------------------------------
typedef struct cmd {
	char** argv;			// Command arguments
	int argc;				// Totatl number of command arguments
	char* in_file;          // Name or path of input file when < redirection is used
    char* out_file;         // Name or path of output file when > or >> redirection is used
    bool append;    		// True if >> redirection
    bool stderr;			// True if 2> redirection (honors only)
	struct cmd* next_node;	// Next command in the linkedlist
} cmd_t;

typedef struct {
	char* user_input;			// User input
	unsigned int total_cmd_t;	// Total number of commands
	cmd_t* head_node;			// First command in a linkedlist
} input_struct;

// ---------------------------------------
// The input_struct* shell points to the
// input_struct and its associated cmd_t
// structures, which together form a linked 
// list that is an internal representation
// of the user input.
extern input_struct* shell;


// ----------------------------------------
// Functions that CANNOT be modified
// ----------------------------------------

// ------------------------------------
// Removes leading and trailing whitespace
// from a string.
//
// This function is useful when parsing
// and cleaning user input.
void trim( char* str );


// ------------------------------------
// Executes a built-in command.
//
// This function handles built-in commands only,
// specifically `cd` and `exit`. These commands
// are executed directly by the shell process
// rather than by creating a child process.
//
// See the "Shell" section in the README.
void builtin( cmd_t* cmd );


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
bool is_builtin( cmd_t* cmd );

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
void execute();

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
int parse_input( char* user_input );

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
void parse_command( char* command );

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
void unallocate_resources();


// --------------------------------------
// This function is provided for debugging
// purposes.
//
// Example code is included, but you are
// encouraged to modify it as needed to help
// you and your team debug and inspect the
// internal representation of the parsed
// user input.
void debug();
