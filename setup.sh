#!/bin/bash

USER=frederikrosenberg
QMK_FIRMWARE_DIR="${QMK_FIRMWARE_DIR:-$HOME/qmk_firmware}"
KYRIA_REVISION="${KYRIA_REVISION:-rev1}"

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

mkdir -p "$QMK_FIRMWARE_DIR/users"
mkdir -p "$QMK_FIRMWARE_DIR/keyboards/splitkb/kyria/$KYRIA_REVISION/keymaps"

ln -sfn "$SCRIPT_DIR/user/" "$QMK_FIRMWARE_DIR/users/$USER"
ln -sfn "$SCRIPT_DIR/keyboards/splitkb/kyria/" "$QMK_FIRMWARE_DIR/keyboards/splitkb/kyria/$KYRIA_REVISION/keymaps/$USER"
