#!/bin/bash

# ./tar_script.sh work_dir

if [ -z "$1" ]; then
    SOURCE_DIR="work_dir"
else
    SOURCE_DIR="$1"
fi

BASE_ZIP_DIR="zip_files"

if [ ! -d "$BASE_ZIP_DIR" ]; then
    NEXT_VER=1
else
    LAST_VER=$(ls "$BASE_ZIP_DIR" | grep -E '^v[0-9]+$' | sed 's/v//' | sort -n | tail -1)
    NEXT_VER=$((LAST_VER + 1))
fi

TARGET_DIR="$BASE_ZIP_DIR/v$NEXT_VER"
mkdir -p "$TARGET_DIR"

tar -czvf "$TARGET_DIR/assotnik_p2.tar.gz" -C "$SOURCE_DIR" \
    main.cpp Country_Data.cpp Country_Data.hpp Time_Series.cpp Time_Series.hpp assotnik_design_p2.txt passcode.txt Makefile

echo "Archive created in: $TARGET_DIR"