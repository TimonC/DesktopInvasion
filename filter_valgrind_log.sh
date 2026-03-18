#!/bin/bash

LATEST_LOG=$(ls -t valgrind_logs/valgrind_*.log 2>/dev/null | head -1)

FILE="${1:-${LATEST_LOG:-valgrind.log}}"
OUTPUT="filtered_log.txt"

if [ ! -f "$FILE" ]; then
    echo "Error: No log file found!"
    echo "Tried: $FILE"
    exit 1
fi

echo "Analyzing: $FILE"
grep "\.cpp:\|\.h:" "$FILE" | grep -v "/usr\|/opt" > "$OUTPUT"
echo "" >> "$OUTPUT"
grep -A2 "LEAK SUMMARY" "$FILE" >> "$OUTPUT"

echo "Saved to $OUTPUT"
cat "$OUTPUT"

