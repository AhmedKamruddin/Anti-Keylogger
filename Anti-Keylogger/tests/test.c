#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/input.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/*
* A single keydown or keyup generates three events - EV_MSC, EV_KEY, and EV_SYN
* with event value 1 for keydown or 0 for keyup. A continous keypress generates 
* two events - EV_KEY and EV_SYN with event value 2.
* Once this program starts executing, the program should grab /dev/input/eventX 
* only after all the last keypress events have been sent through, ie. the last 
* event to be read and thrown away is EV_SYN and value is 0.
*/
void evdev_grab_device(struct libevdev *dev, struct input_event ev)
{
    do {
        libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, &ev);
    } while ( !(libevdev_event_is_type(&ev, EV_SYN) && ev.value==0) );
    libevdev_grab(dev, LIBEVDEV_GRAB);
}


int encrypt(unsigned short code)
{
    int key = 1;
    unsigned short encrypted_keycode;
    encrypted_keycode = code + key;
    return encrypted_keycode;
}

void module1() {
    
}

int main()
{
    // Initialize variables
    int err;
    int fd, uifd;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;    
    struct input_event ev;   
    Display *display;
    Window root, focused_window;
    XEvent event;
    KeySym key_sym;
    char key_char;
    unsigned short encrypted_keycode, decrypted_keycode;
    int key = 1;
    int revert_to_return;


    /*
    * Setup device and file descriptor handling
    * NOTE TO SELF: Need to extend on error handling of file descriptors
    */
    fd = open("/dev/input/event5", O_RDONLY);
    if (fd < 0) {
        perror("Error: Could not open /dev/input/eventX\n");
        return -1;
    }
    err = libevdev_new_from_fd(fd, &dev);
    if (err != 0) {
        perror("Error: Could not create device descriptor\n");
        return -1;
    }
    uifd = open("/dev/uinput", O_RDWR);
    if (uifd < 0) {
        perror("Error: Could not open /dev/uinput\n");
        return -1;
    }
    err = libevdev_uinput_create_from_device(dev, uifd, &uidev);
    if (err != 0) {
        perror("Error: Could not create uidevice descriptor\n");
        return -1;
    }

    // Open a connection to the X server
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        perror("Error: Could not open X display\n");
        return -1;
    }

    // Get the root window
    root = DefaultRootWindow(display);

    evdev_grab_device(dev, ev);

    XGetInputFocus(display, &focused_window, &revert_to_return);

    int i = 0;
    while (i < 6) {
        // Read event in device
        libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, &ev);        
        
        // If the event is of type EV_KEY -
        if (libevdev_event_is_type(&ev, EV_KEY)) {
            printf("EV_KEY event read from keyboard:\t%s %s %d\n", libevdev_event_type_get_name(ev.type), libevdev_event_code_get_name(ev.type, ev.code), ev.value);
           
            // Encrypt keystroke
            ev.code = encrypt(ev.code);
            
            /*
            * Write the encrypted keystroke back to the system through /dev/uinput. 
            * Terminate the event sequence with EV_SYN/SYN_REPORT/0. Otherwise listeners on 
            * the device node will not see the event until the next EV_SYN is posted.
            */
            libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
            libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);

            XGrabKey(display, AnyKey, 0, focused_window, True, GrabModeAsync, GrabModeAsync);    
            XNextEvent(display, &event);
            XUngrabKey(display, AnyKey, 0, focused_window);

            printf("Encrypted EV_KEY written to uinput:\t%s %d\n", libevdev_event_code_get_name(ev.type, ev.code), ev.value);
            
            printf("\t\tModule 1 completed\n");
            printf("------------------------------------------------------------\n");
            

            XLookupString(&event.xkey, &key_char, 1, &key_sym, NULL);
            printf("Encrypted keystroke read:\t\t%c (Keysym: 0x%lx) %d\n", key_char, key_sym, event.type == KeyPress ? 1:0);
            
            encrypted_keycode = XKeysymToKeycode(display, key_sym) - 8;
            decrypted_keycode = (encrypted_keycode - key) + 8;
            event.xkey.keycode = decrypted_keycode;
        

            if (event.type == KeyPress)
                XSendEvent(display, focused_window, True, KeyPressMask, &event);
            else if (event.type == KeyRelease)
                XSendEvent(display, focused_window, True, KeyReleaseMask, &event);

            XLookupString(&event.xkey, &key_char, 1, &key_sym, NULL);
            printf("Decrypted keystroke sent:\t\t%c (Keysym: 0x%lx) %d\n", key_char, key_sym, event.type == KeyPress ? 1:0);
            XSync(display, True);

            printf("\t\tModule 2 completed\n");
            printf("############################################################\n\n");

            i++;
        }

    }

    libevdev_grab(dev, LIBEVDEV_UNGRAB);

    XCloseDisplay(display);

    // Clean up device and file descriptors
    libevdev_uinput_destroy(uidev);
    libevdev_free(dev);
    close(uifd);
    close(fd);
    return 0;
}
