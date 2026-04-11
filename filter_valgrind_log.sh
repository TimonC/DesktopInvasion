#!/bin/bash
LATEST_LOG=$(ls -t valgrind_logs/valgrind_*.log 2>/dev/null | head -1)
FILE="${1:-${LATEST_LOG:-valgrind.log}}"
OUTPUT="filtered_log.txt"

if [ ! -f "$FILE" ]; then
    echo "Error: No log file found!"
    exit 1
fi

echo "Analyzing: $FILE"
> "$OUTPUT"

grep "ERROR SUMMARY\|LEAK SUMMARY\|definitely lost\|indirectly lost" "$FILE" | tail -6 >> "$OUTPUT"

echo "" >> "$OUTPUT"
awk '
/^==.*== [A-Z]/ { block = $0; next }
/\.cpp:|\.h:/ && !/\/usr|\/opt/ { print block; print; block = ""; count++ }
' "$FILE" | sort -u >> "$OUTPUT"

echo "" >> "$OUTPUT"
awk '
/definitely lost/ { inblock=1; block=$0; next }
inblock && /\.cpp:|\.h:/ && !/\/usr|\/opt/ { print block; print; inblock=0; next }
/^==.*==$/ { inblock=0 }
inblock { block=block"\n"$0 }
' "$FILE" >> "$OUTPUT"

echo "Saved to $OUTPUT"
cat "$OUTPUT"
