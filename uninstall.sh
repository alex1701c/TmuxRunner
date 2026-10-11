#!/bin/bash

# Exit if something fails
set -e

cd build
sudo make uninstall

# KRunner needs to be restarted for the changes to be applied
if pgrep -x krunner > /dev/null
then
    if command -v kquitapp6 > /dev/null; then
        kquitapp6 krunner
    else
        kquitapp5 krunner
    fi
fi

