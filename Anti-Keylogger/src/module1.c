#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <linux/input.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "module1.h"
#include "queue.h"
#include <pthread.h>

/*
* A single keydown or keyup generates three events - EV_MSC, EV_KEY, and EV_SYN
* with event value 1 for keydown or 0 for keyup. A continous keypress generates 
* two events - EV_KEY and EV_SYN with event value 2.
* Once this program starts executing, the program should grab /dev/input/eventX 
* only after all the last keypress events have been sent hrough, ie. the last 
* event to be read and thrown away is EV_SYN and value is 0.
*/
void evdev_grab_device(struct libevdev *dev, struct input_event ev)
{
    do {
        libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, &ev);
    } while ( !(libevdev_event_is_type(&ev, EV_SYN) && ev.value==0) );
    libevdev_grab(dev, LIBEVDEV_GRAB);
}


int encrypt(int urandom_fd, unsigned short code)
{
    unsigned char buffer;
    unsigned short key, encrypted_keycode;

    do {
        read(urandom_fd, &buffer, sizeof(buffer));
        key = buffer >> 2;
        encrypted_keycode = ((code ^ key) % 53) + 1;
    } while (encrypted_keycode == 29 || encrypted_keycode == 42);

    enqueue(key_queue, key);
    printf("In module 1, key written to key_queue is %d\n", key);    

    return encrypted_keycode;
}

// driver function
void* module1()
{
    // Initialize variables
    int err;
    int fd, uifd, urandom_fd;
    struct libevdev *dev;
    struct libevdev_uinput *uidev;    
    struct input_event ev;

    /*
    * Setup device and file descriptor handling
    * NOTE TO SELF: Need to extend on error handling of file descriptors
    */
    fd = open("/dev/input/event5", O_RDONLY);
    if (fd < 0) {
        perror("Error: Could not open /dev/input/eventX\n");
        return NULL;
    }
    
    err = libevdev_new_from_fd(fd, &dev);
    if (err != 0) {
        perror("Error: Could not get evdev device\n");
        return NULL;
    }

    uifd = open("/dev/uinput", O_RDWR);
    if (uifd < 0) {
        perror("Error: Could not open /dev/uinput\n");
        return NULL;
    }

    err = libevdev_uinput_create_from_device(dev, uifd, &uidev);
    if (err != 0) {
        perror("Error: Could not get uinput device\n");
        return NULL;
    }

    urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        perror("Error: Could not open /dev/urandom\n");
        return NULL;
    }
    
    evdev_grab_device(dev, ev);

    /*
    * Read 25 keystrokes. In the release version, this should be an infinite loop
    * and device should be ungrabbed only when the program quits.
    */
    int i = 0;
    while (i < 10) {

        libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, &ev);
        // If the event is of type EV_KEY 
        if (libevdev_event_is_type(&ev, EV_KEY)) {
            pthread_mutex_lock (&mutex);
            // printf("Thread 1 locked\n");

            printf("EV_KEY event read from keyboard:\t%s %s keycode=%d %d\n", libevdev_event_type_get_name(ev.type), libevdev_event_code_get_name(ev.type, ev.code), ev.code, ev.value);

            // Encrypt keystroke
            if (1 <= ev.code && ev.code <=53 && ev.code!=29 && ev.code!=42)
                ev.code = encrypt(urandom_fd, ev.code);
            
            /*
            * Write the encrypted keystroke back to the system through /dev/uinput. 
            * Terminate the event sequence with EV_SYN/SYN_REPORT/0. Otherwise listeners on 
            * the device node will not see the event until the next EV_SYN is posted.
            */
            libevdev_uinput_write_event(uidev, ev.type, ev.code, ev.value);
            libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0);
            
            is_module1_done = 1;
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            // printf("Thread 1 unlocked\n");

            printf("Encrypted EV_KEY(%d) written to uinput:\t%s %d\n", ev.code, libevdev_event_code_get_name(ev.type, ev.code), ev.value);

            printf("\t\tModule 1 completed\n");
            // printf("------------------------------------------------------------\n");
            i++;
        }
        
        
                 
    }
    libevdev_grab(dev, LIBEVDEV_UNGRAB);
    

    // Clean up device and file descriptors
    libevdev_uinput_destroy(uidev);
    libevdev_free(dev);
    close(uifd);
    close(fd);
    return NULL;
}