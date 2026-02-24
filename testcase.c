#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

// ----------------------------------
// You may add helper functions here
// -----------------------------------


int verify_node(cmd_t* node, char** exp_argv, int exp_argc, char* exp_in, char* exp_out, bool exp_append, bool exp_stderr){

    if (node==NULL){
        printf("node: node is NULL");
        return FAIL;
    }

    for (int i=0; i<exp_argc; i++){
        if (strcmp(node->argv[i], exp_argv[i]) != 0) {
            printf("argv: argv[%d]=%s when it should have been %s", i, node->argv[i], exp_argv[i]);
            return FAIL;
        }
    }

    if ((node->in_file == NULL && exp_in != NULL) || 
        (node->in_file != NULL && exp_in == NULL) ||
        (strcmp(node->in_file, exp_in) != 0)){
            printf("infile: epxected %s got %s\n", exp_in, node->in_file);
    }   
    
    if ((node->out_file == NULL && exp_out != NULL) || 
        (node->out_file != NULL && exp_out == NULL) ||
        (strcmp(node->out_file, exp_out) != 0)){
            printf("outfile: epxected %s got %s\n", exp_out, node->out_file);
    }   
    
    
    if(node->argc != exp_argc){
        printf("argc: expected %d got %d\n", exp_argc, node->argc);
        return FAIL;
    }

    if(exp_append != node->append) {
        printf("append: expected %d got %d\n", exp_append, node->append);
        return FAIL;
    }

    if(exp_stderr != node->stderr) {
        printf("stderr: expected %d got %d\n", exp_stderr, node->stderr);
        return FAIL;
    }

    if (node->argv[exp_argc] != NULL){
        printf("argv: is not null terminated.");
        return FAIL;
    }

    return PASS;
}

int run_test(char* input, cmd_t* exp_list, int exp_num) {
    int rv = PASS;

    char* user_input = (char*)calloc(CHUNK, sizeof(char));
    strncpy(user_input, input, CHUNK - 1);

    parse_input(user_input);

    if (shell->total_cmd_t == 0){
        printf("shell not init'd");
        rv = FAIL;
    }
    else if (shell->total_cmd_t != exp_num) {
        printf("shell expected %d commands got %d\n", exp_num, shell->total_cmd_t);
        rv = FAIL;
    } else {
        cmd_t* current = shell->head_node;
        for (int i = 0; i < exp_num; i++) {
            printf("Verifying command %d %s   \n", i, exp_list[i].argv[0]);
            if (verify_node(current, exp_list[i].argv, exp_list[i].argc, exp_list[i].in_file, 
                        exp_list[i].out_file, exp_list[i].append, exp_list[i].stderr) == FAIL) {
                rv = FAIL;
                break;
            }
            current = current->next_node;
        }
    }

    unallocate_resources();
    free(user_input);

    return rv;
}
// End helper function definitions


typedef int (*function_ptr)();

typedef struct {
    char *name;
    function_ptr fn;
} fn_table_entry_t;


// ----------------------------------
// Define test cases here.
// Three examples have been provided.
// ----------------------------------
int tc1() {
    char* input = "ls | grep rizz > out.txt";

    cmd_t expected[2] = {
        { .argv = (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL},
        { .argv = (char*[]){"grep", "rizz", NULL}, .argc = 2, .in_file = NULL, .out_file = "out.txt" }
    };

    return run_test(input, expected, 2);
}

// null input (i.e. press enter at the shell)
int tc2() {
    char* input = NULL;

    cmd_t* expected = NULL;

    return run_test(input, expected, 0);
}
// one call (check total_cmd_t = 1)
int tc3() {
    char* input = "ls";

    cmd_t expected[1] = {
        { .argv= (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 1);
}
// two call (check total_cmd_t = 2)
int tc4() {
    char* input = "ls | grep .txt";

    cmd_t expected[2] = {
        { .argv= (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL } ,
        { .argv= (char*[]){"grep", ".txt", NULL}, .argc=2, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 2);
}
// argument (ls -l) (ensure argc=2 and total_cmd_t is still 1)
int tc5() {
    char* input = "ls -l";

    cmd_t expected[1] = {
        { .argv= (char*[]){"ls", "-l", NULL}, .argc=2, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 1);
}
// strings (echo "hello world" has argc=2) 
int tc6() {
    char* input = "echo \"hello world\" ";

    cmd_t expected[1] = {
        { .argv= (char*[]){"echo", "\"hello world\"", NULL}, .argc=2, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 1);
}

// TODO
// output redirect (ls > file.txt)
// input redirect (wc -l < file.txt)
// append redirect (echo "hello" >> log.txt)
// no whitespace still grammatical (ls>file.txt)
// single pipe (ls | grep .txt)
// double pipe (ls | grep .txt | wc -l)
// pipe with argc>1 (ls -l | grep .txt)
// redirect after pipe (ls -l | grep .txt > list.txt)
// middle redirect (ls -l | grep .txt > list.txt | wc -l)
// tokens inside strings (echo "a pipe symbol is | and 2>1")


// End test case function definitions

// ----------------------------------
// Every new test case function needs 
// to be defined the function table.
// Update as needed.
// ----------------------------------

fn_table_entry_t fn_table[] = {
    {"tc1", tc1},
    {"tc2", tc2},
    {"tc3", tc3},
    {"tc4", tc4},
    {"tc5", tc5},
    {"tc6", tc6},
    {NULL, NULL} // mark the end
};

// End function table definitions

function_ptr lookup_function(const char *fn_name) {
    if (!fn_name) {
        return NULL;
    }
    for (int i = 0; fn_table[i].name != NULL; ++i) {
        if (!strcmp(fn_name, fn_table[i].name)) {
            return fn_table[i].fn;
        }
    }
    return NULL; // testcase not found
}

int main( int argc, char** argv ) {

    int rv = FAIL;
    
    if ( argc != 2 ) {
        printf("------------------------\n");
        printf("Test case program\n");
        printf("------------------------\n");
        printf("Usage: ./testcase <testcase name>\n\n");
        printf("Example: ./testcase tc1\n\n");
        return 0;
    }

    function_ptr func = lookup_function( argv[1] );
    if (func != NULL) {
        rv = func();
    } else {
        printf("Testcase (%s) not defined!\n", argv[1] );
    }

    return rv;
}
