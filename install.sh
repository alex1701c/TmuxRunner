#!/bin/bash

# Exit if something fails
set -e

if [[ $(basename "$PWD") != "TmuxRunner"* ]];then
    git clone https://github.com/alex1701c/TmuxRunner
    cd TmuxRunner/
fi

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
sudo curl https://raw.githubusercontent.com/aplatanado/yakuake-session/master/yakuake-session -o /usr/bin/yakuake-session
sudo chmod +x /usr/bin/yakuake-session

kquitapp5 krunner 2> /dev/null
kstart5 --windowclass krunner krunner > /dev/null 2>&1 &

echo "Installation finished !";
