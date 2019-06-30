#!/bin/bash

if [[ $(basename "$PWD") != "TmuxRunner"* ]];then
    git clone https://github.com/alex1701c/TmuxRunner
    cd TmuxRunner/
fi

mkdir -p build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make -j2
sudo make install
sudo curl https://raw.githubusercontent.com/aplatanado/yakuake-session/master/yakuake-session -o /usr/bin/yakuake-session
sudo chmod +x /usr/bin/yakuake-session

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished !";
