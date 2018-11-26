#!/bin/sh

gcc -std=c99 -Wpedantic -Wall -Wextra -D_POSIX_C_SOURCE=200809L -o usr types.c proto.c net.c actions.c main.c
