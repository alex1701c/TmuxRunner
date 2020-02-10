#!/bin/bash

# Exit if something fails
set -e

if [[ $(basename "$PWD") != "TmuxRunner"* ]];then
    git clone https://github.com/alex1701c/TmuxRunner
    cd TmuxRunner/
fi

# Create folders
mkdir -p build
mkdir -p ~/.local/lib/qt/plugins
mkdir -p ~/.local/share/kservices5
mkdir -p ~/.local/bin/

cd build

# Add the installation path to the QT_PLUGIN_PATH
if [[ -z "${QT_PLUGIN_PATH}" || "${QT_PLUGIN_PATH}" != *".local/lib/qt/plugins/"* ]]; then
    echo "export QT_PLUGIN_PATH=~/.local/lib/qt/plugins/:$QT_PLUGIN_PATH" >> ~/.bashrc
    export QT_PLUGIN_PATH=~/.local/lib/qt/plugins/:$QT_PLUGIN_PATH
fi

cmake -DKDE_INSTALL_QTPLUGINDIR="~/.local/lib/qt/plugins" -DKDE_INSTALL_KSERVICES5DIR="~/.local/share/kservices5" -DCMAKE_BUILD_TYPE=Release ..

make -j$(nproc)
make install

# Script for yakuake interaction
curl https://raw.githubusercontent.com/aplatanado/yakuake-session/master/yakuake-session -o ~/.local/bin/yakuake-session
chmod +x ~/.local/bin/yakuake-session

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished !";
