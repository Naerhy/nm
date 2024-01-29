#!/bin/bash

if [ "$#" -eq 0 ]; then
	echo "Please provide at least 1 file as argument"
	exit 1
fi

readonly file1="ft_nm_output"
readonly file2="nm_output"

for arg in "$@"; do
	echo -e "\e[33mrunning nm commands on file $arg:\e[0m"
	./ft_nm "$arg" &> "$file1"
	nm -p "$arg" &> "$file2"
	diff "$file1" "$file2"
	rm "$file1" "$file2"
done
