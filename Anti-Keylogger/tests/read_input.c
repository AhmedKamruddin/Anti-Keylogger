#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include<linux/input.h>

int main() {
    int uinput_fd;

    // Open the /dev/uinput file for reading
    uinput_fd = open("/dev/uinput", O_RDONLY);
    if (uinput_fd == -1) {
        perror("Failed to open /dev/uinput");
        return 1;
    }

    // Read input events in a loop
    while (1) {
        struct input_event event;

        // Read an input event
        ssize_t bytesRead = read(uinput_fd, &event, sizeof(struct input_event));

        if (bytesRead == -1) {
            //perror("Error reading from /dev/uinput");
            continue;
        }

        if (bytesRead == sizeof(struct input_event)) {
            // Process the input event here
            printf("Event type: %u, code: %u, value: %d\n",
                   event.type, event.code, event.value);
        }
    }

    // Close the /dev/uinput file
    close(uinput_fd);

    return 0;
}
