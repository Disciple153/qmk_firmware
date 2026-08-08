#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ANIMS_DIR="$SCRIPT_DIR/../../../../../quantum/rgb_matrix/animations"
EFFECTS_INC="$ANIMS_DIR/rgb_matrix_effects.inc"
CONFIG_H="$SCRIPT_DIR/config.h"
USER_INC="$SCRIPT_DIR/rgb_matrix_user.inc"

id=0
echo "$id RGB_MATRIX_NONE"; ((id++))
echo "$id RGB_MATRIX_SOLID_COLOR"; ((id++))

while IFS= read -r line; do
    header=$(echo "$line" | grep -oP '(?<=#include ")[^"]+')
    [[ -z "$header" ]] && continue
    while IFS= read -r effect_line; do
        name=$(echo "$effect_line" | grep -oP '(?<=RGB_MATRIX_EFFECT\()[^)]+')
        [[ -z "$name" ]] && continue
        if grep -q "ENABLE_RGB_MATRIX_${name}" "$CONFIG_H"; then
            echo "$id RGB_MATRIX_${name}"; ((id++))
        fi
    done < "$ANIMS_DIR/$header"
done < "$EFFECTS_INC"

while IFS= read -r line; do
    name=$(echo "$line" | grep -oP '(?<=RGB_MATRIX_EFFECT\()[^)]+')
    [[ -z "$name" ]] && continue
    echo "$id RGB_MATRIX_CUSTOM_${name}"; ((id++))
done < <(grep 'RGB_MATRIX_EFFECT(' "$USER_INC")
