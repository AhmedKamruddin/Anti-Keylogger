#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <fcntl.h>
#include "queue.h"
#include "module2.h"

/*
* A single keystroke generates two event types - KeyPress and KeyRelease. This
* program should grab keyboard only after the last keypress has been sent 
* through, ie. the last event to be read and thrown away is KeyRelease. 
*/
void xorg_grab_device(Display *display, Window root, Window focused_window)
{
    XEvent event;
    XSelectInput(display, focused_window, KeyPressMask | KeyReleaseMask);
    do {
        XNextEvent(display, &event);
    } while (event.type != KeyRelease);
    XGrabKeyboard(display, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);
}

unsigned short decrypt(Display *display, KeySym key_sym)
{
    unsigned short encrypted_keycode, decrypted_keycode;
    unsigned short key = dequeue(key_queue);
    printf("In module 2, key is %d\n", key);

    encrypted_keycode = XKeysymToKeycode(display, key_sym) - 8;
    decrypted_keycode = ((encrypted_keycode - 1) ^ key) + 8;
    return decrypted_keycode;
}

// driver function
void* module2() {
    Display *display;
    Window root, focused_window;
    XEvent event;
    KeySym key_sym;
    char key_char;
    
    int key = 1;
    int revert_to_return;

    // Open a connection to the X server
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Error: Could not open X display\n");
        return NULL;
    }

    // Get the root window
    root = DefaultRootWindow(display);

    XGetInputFocus(display, &focused_window, &revert_to_return);
    // printf("Id of focused window: 0x%lx\n", focused_window);

    // Grab the keyboard
    xorg_grab_device(display, root, focused_window);

    int i = 0;
    while (i < 10) {
        pthread_mutex_lock(&mutex);
        // printf("Thread 2 before cv\n");
        while (!is_module1_done) {
            pthread_cond_wait(&cond, &mutex);
        }
        
        // printf("Thread 2 locked after cv\n");
        // XGrabKey(display, AnyKey, 0, focused_window, True, GrabModeAsync, GrabModeAsync);    
        XNextEvent(display, &event);
        // XUngrabKey(display, AnyKey, 0, focused_window);
        
        XLookupString(&event.xkey, &key_char, 1, &key_sym, NULL);
        printf("Encrypted keystroke read:\t\t%c %d (Keysym: 0x%lx) %d\n", key_char, event.xkey.keycode, key_sym, event.type == KeyPress ? 1:0);
        
        event.xkey.keycode = decrypt(display, key_sym);

        if (event.type == KeyPress)
            XSendEvent(display, focused_window, True, KeyPressMask, &event);
        else if (event.type == KeyRelease)
            XSendEvent(display, focused_window, True, KeyReleaseMask, &event);

        XLookupString(&event.xkey, &key_char, 1, &key_sym, NULL);
        printf("Decrypted keystroke sent:\t\t%c (Keysym: 0x%lx) %d\n", key_char, key_sym, event.type == KeyPress ? 1:0);
        XSync(display, True);
        is_module1_done = 0;
        pthread_mutex_unlock(&mutex);
        // printf("Thread 2 unlocked\n");
        printf("\t\tModule 2 completed\n");

        // printf("############################################################\n\n");
        printf("------------------------------------------------------------\n");
        
        i++;
        
    }

    XUngrabKeyboard(display, CurrentTime);
    XCloseDisplay(display);

    return NULL;
}
