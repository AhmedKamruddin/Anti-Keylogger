#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
int main() {

    /*
    * unsigned short input_event::code = 16 bits = 2 bytes
    * unsigned int XKeyEvent::keycode = 32 bits = 4 bytes
    */
    unsigned char buffer;
    int random_fd;
    int bytes_read;
    unsigned char key;
    unsigned short keycode = 30;
    unsigned short cipher;

    random_fd = open("/dev/urandom", O_RDONLY);
    if (random_fd < 0) {
        perror("Error: Failed to open /dev/urandom\n");
        return -1;
    }

    int i = 0;
    while (i < 20) {

        keycode = rand() % 54;
        printf("Keycode is %d\n", keycode);
        if (keycode != 29 || keycode != 42)
        {
            /*
            *      Key          Keycode
            * KEY_LEFTCTRL        29
            * KEY_LEFTSHIFT       42
            * The keycode to be encrypted is in the range [1, 53] - {29, 42}, ie. 6
            * bits of data. A byte is read from /dev/urandom and is shifted 2 bits 
            * to the right to get 6 bits of data that lies is in [0, 63]. 
            * (keycode ^ key) gives ciphertext in [0, 63]
            * ((keycode ^ key) modulo 52) + 1 gives cipher in [1, 53]
            * If ciphertext is in {29, 43}, repeat the process starting from 
            * read /dev/urandom.
            */
            do {
                bytes_read = read(random_fd, &buffer, sizeof(buffer));
                key = buffer >> 2;
                printf("Key is %d\n", key);
                cipher = ((keycode ^ key) % 53) + 1;
            } while(cipher == 29 || cipher == 42);

            printf("Cipher is %d\n", cipher);
            printf("Decrypted cipher is %d\n\n", (cipher - 1) ^ key);
        }
        i++;
    }
    return 0;
}