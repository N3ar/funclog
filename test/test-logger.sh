#!/bin/bash
clang test-logger.c -llogger -o test-logger && ./test-logger

echo "Contents of logfile:"
cat logtest.txt
