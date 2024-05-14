#!/bin/bash


COMMAND="./neon_0t"
echo "Running benchmark for $COMMAND"

VALGRIND_OUTPUT=$(valgrind --log-file=valgrind_log.txt $COMMAND)
MEMUSAGE=$(grep -E "HEAP SUMMARY:|in use at exit:|total heap usage:" valgrind_log.txt)
rm valgrind_log.txt
echo "$MEMUSAGE"
time $COMMAND
echo ""

exit 0
