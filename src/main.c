#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int ac, char **av){

    /*
        - connect to the esp-displayer
        - identify as 3020-NANO
        - execute the stty command to change the baudrate to 9600
        - open /dev/ttyATH0 for both reading and writing.
        - open /dev/input/event0 for reading.
        - open socket with the esp32
        - use poll to not block the IO operations.
        - read from the keyboard and send for both the esp32 and the nano.
        - for the esp32 I need to attach a '/print ' first.
        - everything will be lowercase for now.
    */
}