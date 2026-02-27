#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

#include <fcntl.h>
#include <unistd.h>

// ----------------------------------
// You may add helper functions here
// -----------------------------------
int verify_file(char* path, int append, char *exp){
    if (!path || !exp){
        return FAIL;
    }
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return FAIL;
    }

    // hacky but will do the trick for anyone with 5KB free heap memory!
    char* buf = (char*)calloc(1, 5001);

    if(!buf){
        close(fd);
        return FAIL;
    }
    
    int rn = read(fd, buf, 5000);
    close(fd);
    if(rn <0){ 
        free(buf);
        return FAIL;
    }
    
    buf[rn] = '\0';

    int ok = PASS;
    int lexp = strlen(exp);
    printf("read %s \n expecting %s \n", buf, exp);
    if (append) {
        // NTS googled for this but strstr is java indexOf https://man7.org/linux/man-pages/man3/strstr.3.html 
        ok = (strstr(buf, exp) != NULL);
    } else {
        if (rn >= lexp) {
            ok = (strncmp(buf, exp, lexp) == 0);
        }else{
            ok = FAIL;
        }

    }
    free(buf);
    return ok;

}

int verify_node(cmd_t* node, char** exp_argv, int exp_argc, char* exp_in, char* exp_out, bool exp_append, bool exp_stderr){

    if (node==NULL){
        printf("node: node is NULL");
        return FAIL;
    }

    if (node->argv == NULL) {
        printf("node: node->argv is NULL (parser did not alloc argv )");
        return FAIL;
    }

    for (int i=0; i<exp_argc; i++){
        if (node->argv[i] == NULL){
            printf("exp_argv: argv[%d] is NULL (no arg)", i);
            return FAIL;
        }
        if (exp_argv[i] == NULL){
            printf("exp_argv: argv[%d] is NULL (test is wrong)", i);
            return FAIL;
        }
        if (strcmp(node->argv[i], exp_argv[i]) != 0) {
            printf("argv: argv[%d]=%s when it should have been %s", i, node->argv[i], exp_argv[i]);
            return FAIL;
        }
    }

    if ((node->in_file == NULL && exp_in != NULL) || 
        (node->in_file != NULL && exp_in == NULL) ||
        (node->in_file != NULL && exp_in != NULL &&
        strcmp(node->in_file, exp_in) != 0)){
            printf("infile: epxected %s got %s\n", exp_in, node->in_file);
            return FAIL;
    }   
    
    if ((node->out_file == NULL && exp_out != NULL) || 
        (node->out_file != NULL && exp_out == NULL) ||
        (node->out_file != NULL && exp_out != NULL &&
        strcmp(node->out_file, exp_out) != 0)){
            printf("outfile: epxected %s got %s\n", exp_out, node->out_file);
            return FAIL;
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

int run_test(char* input, cmd_t* exp_list, int exp_num, char* exp_output) {
    int rv = PASS;

    char* user_input = (char*)calloc(CHUNK, sizeof(char));
     if (input) {
        strncpy(user_input, input, CHUNK - 1);
    } else {
    
        user_input[0] = '\0';
    }
    

    parse_input(user_input);
    execute(); 
    if (shell == NULL){
        printf("shell is NULL");
        rv = PASS;
    }
    else if (shell->total_cmd_t == 0){
        printf("shell: no commands");
        rv = PASS;
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

            if (exp_output && exp_list[i].out_file) {
                if (!verify_file(exp_list[i].out_file,
                                            exp_list[i].append,
                                            exp_output)) {
                    printf("output file %s does not contain expected data\n",
                           exp_list[i].out_file);
                    rv = FAIL;
                    break;
                }
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
    char* input = "ls | grep README.md > out.log";

    cmd_t expected[2] = {
        { .argv = (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL},
        { .argv = (char*[]){"grep", "README.md", NULL}, .argc = 2, .in_file = NULL, .out_file = "out.log" }
    };

    return run_test(input, expected, 2, "README.md");
}

// null input (i.e. press enter at the shell)
int tc2() {
    char* input = NULL;

    cmd_t* expected = NULL;

    return run_test(input, expected, 0, NULL);
}
// one call (check total_cmd_t = 1)
int tc3() {
    char* input = "ls";

    cmd_t expected[1] = {
        { .argv= (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 1, NULL);
}
// two call (check total_cmd_t = 2)
int tc4() {
    char* input = "ls | grep .log";

    cmd_t expected[2] = {
        { .argv= (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL } ,
        { .argv= (char*[]){"grep", ".log", NULL}, .argc=2, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 2, NULL);
}
// argument (ls -l) (ensure argc=2 and total_cmd_t is still 1)
int tc5() {
    char* input = "ls -l";

    cmd_t expected[1] = {
        { .argv= (char*[]){"ls", "-l", NULL}, .argc=2, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 1, NULL);
}
// strings (echo "hello world" has argc=2) 
int tc6() {
    char* input = "echo \"hello world\" ";

    cmd_t expected[1] = {
        { .argv= (char*[]){"echo", "hello world", NULL}, .argc=2, .in_file=NULL, .out_file=NULL } 
    };

    return run_test(input, expected, 1, NULL);
}

// TODO
// output redirect (ls > file.log)
int tc7() {
    char* input = "ls ./figs > out.log";
    cmd_t expected[1] = {
        { .argv = (char*[]){"ls", "./figs", NULL}, .argc=1, .in_file=NULL, .out_file="out.log" }
    };

    return run_test(input, expected, 1, "ex1.png  ex2.png  ex3.png  ex4.png  ex5.png  ex6.png");
}
// input redirect (wc -l < file.log)
int tc8() {
    //TODO: programmatically create in.log here

    char* input = "wc -l < in.log";
    cmd_t expected[1] = {
        { .argv = (char*[]){"wc", "-l", NULL}, .argc=2, .in_file="in.log", .out_file=NULL }
    };

    return run_test(input, expected, 1, NULL);
}
// append redirect (echo "hello" >> log.log)
int tc9() {
    char* input = "echo \"hello\" >> log.log";
    cmd_t expected[1] = {
        { .argv = (char*[]){"echo", "hello", NULL}, .argc=2, .in_file=NULL, .out_file="log.log", .append=1 }
    };
    return run_test(input, expected, 1, "hello");
}
// no whitespace still grammatical (ls>file.log)
int tc10() {
    char* input = "ls ./figs>out.log";
    cmd_t expected[1] = {
        { .argv = (char*[]){"ls", "./figs", NULL}, .argc=2, .in_file=NULL, .out_file="out.log" }
    };

    return run_test(input, expected, 1, "ex1.png  ex2.png  ex3.png  ex4.png  ex5.png  ex6.png");
}
// single pipe (ls | grep .log)
int tc11() {
    char* input = "ls | grep .log";
    cmd_t expected[2] = {
        { .argv = (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL },
        { .argv = (char*[]){"grep", ".log", NULL}, .argc=2, .in_file=NULL, .out_file=NULL }
    };

    return run_test(input, expected, 2, NULL);
}
// double pipe (ls | grep .log | wc -l)
int tc12() {
    char* input = "ls | grep .log | wc -l";
   
    cmd_t expected[3] = {
        { .argv = (char*[]){"ls", NULL}, .argc=1, .in_file=NULL, .out_file=NULL },
        { .argv = (char*[]){"grep", ".log", NULL}, .argc=2, .in_file=NULL, .out_file=NULL },
        { .argv = (char*[]){"wc", "-l", NULL}, .argc=2, .in_file=NULL, .out_file=NULL }
    };

    return run_test(input, expected, 3, NULL);
}
// pipe with argc>1 (ls -l | grep .log)
int tc13() {
    char* input = "ls -l | grep .log";
   
    cmd_t expected[2] = {
        { .argv = (char*[]){"ls", "-l", NULL}, .argc=2, .in_file=NULL, .out_file=NULL },
        { .argv = (char*[]){"grep", ".log", NULL}, .argc=2, .in_file=NULL, .out_file=NULL }
    };

    return run_test(input, expected, 2, NULL);
}
// redirect after pipe (ls -l | grep .log > list.log)
int tc14() {
    char* input = "echo \"abcdefghijkl\" | grep abcdefghijkl > list.log";
    
    cmd_t expected[2] = {
        { .argv = (char*[]){"echo", "abcdefghijkl", NULL}, .argc=2, .in_file=NULL, .out_file=NULL },
        { .argv = (char*[]){"grep", "abcdefghijkl", NULL}, .argc=2, .in_file=NULL, .out_file="list.log" }
    };

    return run_test(input, expected, 2, "abcdefghijkl");
};
// middle redirect (ls -l | grep .log > list.log | wc -l)
int tc15() {
    char* input = "echo \"abcdefghijkl\" | grep abcdefghijkl > list.log | wc -l";
    
    cmd_t expected[3] = {
        { .argv = (char*[]){"echo", "abcdefghijkl", NULL}, .argc=2, .in_file=NULL, .out_file=NULL },
        { .argv = (char*[]){"grep", "abcdefghijkl", NULL}, .argc=2, .in_file=NULL, .out_file="list.log" },
        { .argv = (char*[]){"wc", "-l", NULL}, .argc=2, .in_file=NULL, .out_file=NULL }
    };

    return run_test(input, expected, 3, "abcdefghijkl.md");
};
// tokens inside strings (echo "a pipe symbol is | and 2>1")
int tc16() {
    char* input = "echo \"a pipe symbol is | and 2>1\"";
    cmd_t expected[1] = {
        { .argv = (char*[]){"echo", "a pipe symbol is | and 2>1", NULL}, .argc=2, .in_file=NULL, .out_file=NULL }
    };
    return run_test(input, expected, 1, NULL);
};


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
    {"tc7", tc7},
    {"tc8", tc8},
    {"tc9", tc9},
    {"tc10", tc10},
    {"tc11", tc11},
    {"tc12", tc12},
    {"tc13", tc13},
    {"tc14", tc14},
    {"tc15", tc15},
    {"tc16", tc16},
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
