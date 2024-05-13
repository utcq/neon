#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 COMMAND"
    exit 1
fi

COMMAND=$*

echo "Memory usage for command: $COMMAND"

PID=$(pgrep -f "$COMMAND")

if [ -z "$PID" ]; then
    echo "Error: Command not found or not running"
    exit 1
fi

pmap -x $PID | grep -v bash | grep -v "locale-archive"

exit 0