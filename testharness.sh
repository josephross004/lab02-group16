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
N=3

for ((i = 1; i < N+1; i++)); do
    echo "----------------------------"
  	./testcase "tc$i"
  	ec=$?
  	test_result "$ec" "tc$i"
 	sleep 1
done

# ----------------------------------------------
# Honors section will included test cases below
# ---------------------------------------------- 

