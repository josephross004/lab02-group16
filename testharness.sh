#!/usr/bin/env bash

test_result() {
	if [ "$1" -eq "0" ]; then
		echo "$2 (PASSED)"
	else
		echo "$2 (FAILED)"
	fi
}

# -------------------------------
# Number of test cases.
# update this number as needed.
# -------------------------------
N=16
# hacky way to clear val.log
echo "" > "val.log"
for ((i = 1; i < N+1; i++)); do
    echo "----------------------------"
	
  	./testcase "tc$i"
  	ec=$?
  	test_result "$ec" "tc$i"
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 --trace-children=yes ./testcase "tc$i" >> "val.log" 2>&1 
	ec=$?
	test_result "$ec" "valgrind tc$i"
 	sleep 1
done

# ----------------------------------------------
# Honors section will included test cases below
# ---------------------------------------------- 

# Process Management: Determine whether the parent process correctly reaps its child processes 
# (hint: inspect process information in the /proc filesystem).

# File descriptor management: Verify that both the parent and child processes correctly close 
# all file descriptors created for redirection and pipe operations. Leaving file descriptors 
# open can cause resource leaks or unexpected behavior. (Hint: you can inspect process 
# information using the /proc filesystem.)

# nonsense with redirects and pipes etc
(echo "ls | grep a > a.log | grep b > b.log | grep c > c.log | grep d >> d.log | grep e >> e.log  | grep f > f.log | grep g | grep h | grep i >> i.log "; sleep 2) | valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./lab02 > proc_n_files.log 2>&1 &

# in theory, can just say SHELL_PID=$!
# HOWEVER valgrind is not what we're looking for - get the pid of the actual shell
VALGRIND_PID=$!

SHELL_PID=""
while [ -z "$SHELL_PID" ]; do
    VSTAT_LINE=$(cat /proc/[0-9]*/stat 2>/dev/null | grep " $VALGRIND_PID ")
    SHELL_PID=""
	# a bash trick - get the first word (space-dlemiited)
	for WORD in $VSTAT_LINE; do
		SHELL_PID=$WORD
		break 
	done
	
done
echo "Looking at shell $SHELL_PID "
# list out all contents of the proc stat files and find Z $SHELL_PID
ZOMBIE_LIST=$(cat  /proc/[0-9]*/stat 2>/dev/null| grep " Z $SHELL_PID ")

ZOMBIE_COUNT=$(cat  /proc/[0-9]*/stat 2>/dev/null| grep -c " Z $SHELL_PID ")

echo " $ZOMBIE_COUNT zombies"
echo "$ZOMBIE_LIST"

# problem i keep running into is that some of the open fds this way are to things like 
# -the proc_n_files.log 
# -/tmp/valgrind_proc...
# -libc.so.6
# which surely aren't actual FDs we need to close manually. So I'm ok with these and filtering them via grep/regex (grep -E)
RAW_OUTPUT=$(ls -l /proc/$SHELL_PID/fd 2>/dev/null | grep -vE "proc_n_files.log|valgrind|libc|dev/pts|debug|vgdb")
# had to google to find -ce grep flags to make it so grep didn't thikk -> was a flag
FD_COUNT=$(echo "$RAW_OUTPUT" | grep -e "->" | wc -l)

echo "found $FD_COUNT fds"
echo "$RAW_OUTPUT"

kill $SHELL_PID 2>/dev/null

exit 0