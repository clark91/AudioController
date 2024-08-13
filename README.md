# AudioController
  Audio Controller

A Simple Windows c++ program that allows the controlling of a specific app's volume using keybinds (Default F22-F24)

## Start with Windows
To start the app with windows open win+R and go to shell:startup 

![image](https://github.com/user-attachments/assets/613a57f3-ff00-42c3-be3c-3d103e0f7590)

and create a shortcut to the .exe

## Config File

Inside config.cfg there are 5 options. 

![image](https://github.com/user-attachments/assets/218ea162-a0c1-4aae-8275-9ff7e08cd1ce)

### ProcessName
```
ProcessName=""
```
This selects the audio session for the specific program that you'd like to control the volume of

### ChangeAmount
```
ChangeAmount=""
```
This changes the amount that the volume changes with each key press

### VolUp
```
VolUp=""
```
Sets the key for the turning the volume up. All key codes follow the value of the virtual keys at https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

### VolDn
```
VolDn=""
```
Sets the key for turning the volume down

### Mute
```
Mute=""
```
Sets the key for muting the program.

## Bugs
If the program seems to not to be working or says suspended in task manager try running as administrator.
