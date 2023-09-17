#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>
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

/*
* Keycode values in Xorg are always greater than keycode values defined by 
* linux kernel by a value of 8. Hence, the arithmetic operations with 8 to 
* account for the difference. 
*/
unsigned short decrypt(Display *display, KeySym key_sym)
{
    unsigned short encrypted_keycode, decrypted_keycode;
    unsigned short key = dequeue(key_queue);
    // printf("Key read for decryption: %d\n", key);

    encrypted_keycode = XKeysymToKeycode(display, key_sym) - 8;
    decrypted_keycode = (encrypted_keycode ^ key) + 8;
    return decrypted_keycode;
}

// driver function
void* module2() {
    Display *display;
    Window root, focused_window;
    XEvent event;
    KeySym key_sym;
    char key_char;

    int revert_to_return;

    // Open a connection to the X server
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Error: Could not open X display\n");
        return NULL;
    }

    root = DefaultRootWindow(display);

    XGetInputFocus(display, &focused_window, &revert_to_return);
    // printf("Id of focused window: 0x%lx\n", focused_window);

    xorg_grab_device(display, root, focused_window);

    /*
    * Read 14 keystrokes. In the release version, this should be an infinite loop.
    */
    int i = 0;
    while (i < 14) {
        
        XNextEvent(display, &event);
        if (event.type == KeyPress) {

            pthread_mutex_lock(&mutex);
            while (!is_module1_done) {
                pthread_cond_wait(&cond, &mutex);
            }
            
            XLookupString(&event.xkey, &key_char, 1, &key_sym, NULL);   // Only necessary for printf statement
            // printf("Encrypted keystroke read from application:\t%c Keycode=%d Value=%d\n", key_char, event.xkey.keycode, event.type == KeyPress ? 1:0);
            
            event.xkey.keycode = decrypt(display, key_sym);

            XSendEvent(display, focused_window, True, KeyPressMask, &event);

            XLookupString(&event.xkey, &key_char, 1, &key_sym, NULL);   // Only necessary for printf statement
            // printf("Decrypted keystroke sent to application:\t%c Keycode=%d Value=%d\n", key_char, event.xkey.keycode, event.type == KeyPress ? 1:0);
            XSync(display, True);


            // printf("\t\tModule 2 completed\n");
            // printf("------------------------------------------------------------\n");
            i++;
            
            is_module1_done = 0;
            pthread_mutex_unlock(&mutex);        
        }    
    }

    XUngrabKeyboard(display, CurrentTime);
    XCloseDisplay(display);

    return NULL;
}
