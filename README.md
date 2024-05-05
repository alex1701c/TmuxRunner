# TmuxRunner

The plugin gets triggered by the word tmux. After that you can search for a session. If the name does not exist it shows
an option to create a new session with the specified name. After the name you can add the initial path. 
But instead of typing out frequently used paths you can use the shortcuts: Each shortcut has to start with a $ and gets replaced with the value. You can use 
absolute paths and relative ones, but if you want to reference a directory in your home folder you can just type
the folder name. For example *Downloads/Bluetooth* gets interpreted as */home/USER/Downloads/Bluetooth*.  
Shortcuts can be created/deleted in the config dialog.

Furthermore you can change the terminal you want tmux to be launched in.
The default options are: Konsole, Yakuake, Terminator and Simple Terminal. You can also define a custom terminal.
In case you want to use one terminal emulator for the mist time and another sometimes you can set one 
as default and select the other for the action (like instance Yakuake in the config screenshot).
Now you can switch between these two the terminals just by Enter(default) or Shift+Enter(action).

If you want to change the terminal for just one command you can add a flag to the end of the statement. 
The flags start with a *-* and is followed by the lowercase start letter if the option. 
For example *-y* opens it in Yakuake and *-c* opens it with your custom terminal.

The option "Show create options for partly matches" allows you to create a session even if any existing session starts with
the query.

Additionally this plugin supports tmuxinator by letting you create new sessions with parameters/options and attach to existing.
You can also combine this with the terminal flags as explained above.  


Required Dependencies
----------------------
Debian/Ubuntu:  
`sudo apt install cmake extra-cmake-modules build-essential libkf5runner-dev libkf5textwidgets-dev qtdeclarative5-dev gettext libkf5notifications-dev libkf5kcmutils-dev wmctrl tmux`

openSUSE:  
`sudo zypper install cmake extra-cmake-modules libQt5Widgets5 libQt5Core5 libqt5-qtlocation-devel ki18n-devel ktextwidgets-devel kconfigwidgets-devel kservice-devel krunner-devel gettext-tools knotifications-devel kcmutils-devel tmux wmctrl`  

Fedora:  
`sudo dnf install cmake extra-cmake-modules kf5-ki18n-devel kf5-kservice-devel kf5-krunner-devel kf5-ktextwidgets-devel gettext kf5-knotifications-devel kf5-kcmutils-devel tmux wmctrl`  

Archlinux(Manjaro):  
`sudo pacman -S cmake extra-cmake-modules knotifications kcmutils tmux wmctrl` 
    
### Build instructions

The easiest way to install is:  
`curl https://raw.githubusercontent.com/alex1701c/TmuxRunner/master/install.sh | bash`  

Or you can do it manually:

```
git clone https://github.com/alex1701c/TmuxRunner
cd TmuxRunner
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_QTPLUGINDIR=$(kf5-config --qt-plugins) ..
make -j$(nproc)
sudo make install
# Optional for yakuake support
sudo curl https://raw.githubusercontent.com/aplatanado/yakuake-session/master/yakuake-session -o /usr/bin/yakuake-session
sudo chmod +x /usr/bin/yakuake-session
```

Restart KRunner to load the runner (in a terminal type: kquitapp5 krunner;kstart5 krunner )

### Configuration
1. Search for "Plasma Search" in Krunner
2. open system settings entry 
3. search for "tmux" in the search field

alternatively, run

```bash
kcmshell5 plasmasearch --args tmuxrunner
```

### Screenshots

#### Simple command and Yakuake as action
![Simple command](https://raw.githubusercontent.com/alex1701c/Screenshots/master/TmuxRunner/overviewtmux.png)

#### Search sessions
![Search sessions](https://raw.githubusercontent.com/alex1701c/Screenshots/master/TmuxRunner/tmux_search_sessions.png)

#### Create session using name, shortcut and flag
![Create session](https://raw.githubusercontent.com/alex1701c/Screenshots/master/TmuxRunner/new_session_with%20path_and_option.png)

#### Config dialog, custom option launches tmux in Konsole tabs instead of new window
![Configure plugin](https://raw.githubusercontent.com/alex1701c/Screenshots/master/TmuxRunner/config_dialog.png)

#### Tmuxinator overview
![Configure plugin](https://raw.githubusercontent.com/alex1701c/Screenshots/master/TmuxRunner/tmuxinator_overview.png)

#### Tmuxinator create session with arguments and options
![Configure plugin](https://raw.githubusercontent.com/alex1701c/Screenshots/master/TmuxRunner/tmuxinator_create_args_options.png)
