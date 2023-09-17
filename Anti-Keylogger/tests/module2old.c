#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <unistd.h>
#include "module2.h"
// Function to get the active window
Window GetActiveWindow(Display *display, Window root) {
    Atom activeWindowProperty = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    Atom actualType;
    int format;
    unsigned long nItems, bytesAfter;
    Window *activeWindow = NULL;

    if (XGetWindowProperty(display, root, activeWindowProperty, 0, 1, False, XA_WINDOW, &actualType, &format, &nItems, &bytesAfter, (unsigned char **)&activeWindow) == Success) {
        if (activeWindow != NULL) {
            Window result = *activeWindow;
            XFree(activeWindow);
            return result;
        }
    }

    return None;
}

void* module2() {
    Display *display;
    Window root, activeWindow;
    XEvent event;
    KeySym keySym;
    char keyChar;
    unsigned short encrypted_keycode, decrypted_keycode;
    int key = 21;

    // Open a connection to the X server
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        fprintf(stderr, "Error: Could not open X display\n");
        return NULL;
    }

    // Get the root window
    root = DefaultRootWindow(display);
    // Event loop
    int i=0;
    while (i<10) {
        activeWindow = GetActiveWindow(display, root);

        

        // Set up keyboard event monitoring for the active window
        if (activeWindow != None) {
    
            XSelectInput(display, activeWindow, KeyPressMask);
            
            XGrabKey(display, AnyKey, 0, activeWindow, True, GrabModeAsync, GrabModeAsync);
            XNextEvent(display, &event);
            XUngrabKey(display, AnyKey, 0, activeWindow);            
            
            if (event.type == KeyPress) {
                XLookupString(&event.xkey, &keyChar, 1, &keySym, NULL);
                printf("Encrypted keystroke read:\t\t%c (KeySym: 0x%lx)\n", keyChar, keySym);

                encrypted_keycode = XKeysymToKeycode(display, keySym) - 8;
                decrypted_keycode = (encrypted_keycode ^ key) + 8;

                event.type = KeyPress;
                event.xkey.keycode = decrypted_keycode;
                event.xkey.state = 0;

                // Convert the keycode to keysym
                XLookupString(&event.xkey, &keyChar, 1, &keySym, NULL);
                printf("Decrypted keystroke written:\t\t%c (KeySym: 0x%lx)\n", keyChar, keySym);

                XSendEvent(display, activeWindow, True, KeyPressMask, (XEvent *)&event);
                XFlush(display);

                event.type = KeyRelease;
                XSendEvent(display, activeWindow, True, KeyReleaseMask, (XEvent *)&event);
                XFlush(display); 

                XNextEvent(display, &event);
                printf("Module 2 completed\n\n");
            }


            i++;
        }

        else {
            // printf("Active window=None.\n");
        }   
    }

    // Close the X server connection (this will not be reached in this example)
    XCloseDisplay(display);

    return NULL;
}
