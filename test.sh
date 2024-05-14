#!/bin/bash

COMMAND="./neon_0t"

# Use pgrep to find the PID of the process associated with the command
PID=$(ps aux | grep "$COMMAND"| awk '{print $2}')
# Print the PID to see if it matches
echo "PID: $PID"

