#!/usr/bin/env bash

gcc -Wall -c -o md5.o md5.c
gcc -Wall -c -o utils.o utils.c
gcc -Wall -Wno-unused-but-set-variable md5.o utils.o -o consumer consumer.c
