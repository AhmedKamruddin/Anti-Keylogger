# Anti-Keylogger
## Developed on
- Arch Linux, kernel version: Linux 6.4.12-arch1-2-g14 x86_64
- X Window System, version number: 11.0, X.org version: 21.1.8
## Requirements
- X11
## Running on your own PC
1. Clone this repository and cd into `Anti-Keylogger` directory.
2. Load uinput module. You only need to do this once in order for /dev/uinput character file to be created.
   ```
   sudo modprobe uinput
   ```
3. Run `sudo evtest` (you may need to install a package for your distribution) on your system and find the device that corresponds to your keyboard, ie. /dev/input/eventX. It is likely that two or more devices show up that are called keyboards. In that case, individually check which event number corresponds to your keyboard through the `evtest` program. The correct one will produce output on screen whenever a key is pressed.
4. Replace "/dev/input/event5" in src/module1.c with your device event number /dev/input/eventX.
5. Compile the program.
   ```
   gcc -levdev -lX11 -Iinclude/ -I/usr/include/libevdev-1.0/ src/anti_keylogger.c src/module1.c src/module2.c src/queue.c -o bin/anti_keylogger
   ```
7. Run.
   ```
   sudo ./bin/anti-keylogger
   ```
8. (Optional)
   - To see the background working of the program, uncomment printf statements in src/module1.c and src/module2.c.
   - The program runs for 14 keystrokes. To increase the number of keystrokes for which the program runs, change the value in the while loop conditions in src/module1.c and src/module2.c. Both the values should be equal.
