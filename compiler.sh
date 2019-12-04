#!/bin/bash

gcc -c ls2mod.c
gcc -c termproject.c
gcc ls2mod.o termproject.o -o termproject
./termproject
