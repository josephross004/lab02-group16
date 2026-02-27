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
