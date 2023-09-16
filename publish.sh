#!/bin/bash

prepend_text="/************************************
*VR474374
*Michele Viviani
*16/09/2023
*************************************/

"

find src/ -type f \( -name "*.c" -o -name "*.h" \) -print0 |
while IFS= read -r -d '' file; do
    sed -i "1s/^/$prepend_text/" "$file"
done

find inc/ -type f \(-name "*.h" \) -print0 |
while IFS= read -r -d '' file; do
    sed -i "1s/^/$prepend_text/" "$file"
done

echo "Done"
