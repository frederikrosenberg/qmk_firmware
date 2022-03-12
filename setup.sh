#!/bin/bash

USER=frederikrosenberg

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

ln -sf "$SCRIPT_DIR/user/" "$HOME/qmk_firmware/users/$USER"
ln -sf "$SCRIPT_DIR/keyboards/splitkb/kyria/" "$HOME/qmk_firmware/keyboards/splitkb/kyria/keymaps/$USER"
