# TmuxRunner

Required Dependencies
----------------------
Debian/Ubuntu:  
`sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext wmctrl`

openSUSE  
`sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kservice-devel krunner-devel gettext-tools`  

Fedora  
`sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext`  

### Build instructions

The easiest way to install is:  
`curl https://raw.githubusercontent.com/alex1701c/TmuxRunner/master/install.sh | bash`

Or you can do it manually:

```
git clone https://github.com/alex1701c/TmuxRunner
cd TmuxRunner
mkdir -p build
cd build
cmake -DQT_PLUGIN_INSTALL_DIR=`kf5-config --qt-plugins` ..
make
sudo make install
sudo curl https://raw.githubusercontent.com/aplatanado/yakuake-session/master/yakuake-session -o /usr/bin/yakuake-session
sudo chmod +x /usr/bin/yakuake-session
```

Restart krunner to load the runner (in a terminal type: kquitapp5 krunner;kstart5 krunner )

The plugin gets triggered by the word tmux. After that you can search for a session. If the name does not exist it shows
an option to create a new session with the specified name. After the name you can add the initial path. 
But instead of typing out frequently used paths you can use the shortcuts: Each shortcut has to start with a $ and gets replaced with the value. You can use 
absolute paths and relative ones, but if you want to reference a directory in your home folder you can just type
the folder name. For example *Downloads/Bluetooth* gets interpreted as */home/USER/Downloads/Bluetooth*. 

Furthermore you can change the terminal you want tmux to be launched in.
The default options are: Konsole, Yakuake, Terminator and Simple Terminal. You can also define a custom terminal.
If you want to change the terminal for just one command you can add a flag to the end of the statement. 
The flags start with a *-* and is followed by the lowercase start letter if the option. 
For example *-y* opens it in Yakuake and *-c* opens it with your custom terminal.

The option "Show create options for partly matches" allows you to create a session even if any existing session starts with
the query. This option is enabled in the screenshots (last run option in first two screenshots).

### Screenshots

#### Simple command
![Simple command](https://raw.githubusercontent.com/alex1701c/TmuxRunner/master/screenshots/overviewtmux.png)

#### Search sessions
![Search sessions](https://raw.githubusercontent.com/alex1701c/TmuxRunner/master/screenshots/tmux_search_sessions.png)

#### Create session using name, shortcut and flag
![Create session](https://raw.githubusercontent.com/alex1701c/TmuxRunner/master/screenshots/new_session_with%20apth_and_option.png)

#### Config dialog, custom option launches tmux in Konsole tabs instead of new window
![Configure plugin](https://raw.githubusercontent.com/alex1701c/TmuxRunner/master/screenshots/config_dialog.png)
