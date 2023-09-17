gcc -levdev -lX11 -Iinclude/ -I/usr/include/libevdev-1.0/ src/anti_keylogger.c src/module1.c src/module2.c -o bin/anti_keylogger
sudo bin/anti_keylogger
