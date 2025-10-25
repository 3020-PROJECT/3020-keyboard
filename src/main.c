#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>


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

void    exitWithError(char *error){
    perror(error);
    exit(1);
}

int connectToDisplay(char *display_ip, char *display_port){
    return -1;
}

int openSerialCommunication(void){
    int fd;
    struct termios tty;

    
    fd = open("/dev/ttyATH0", O_RDWR | O_NOCTTY);
    if (fd < 0)
        exitWithError("open /dev/ttyATH0");
    

    // Get current settings:
    if (tcgetattr(fd, &tty) != 0) {
        close(fd);
        exitWithError("unable to get the /dev/ttyATH0 tty settings");
    }

    // setting the baudrate:
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);

    // "raw" mode roughly means:
    tty.c_cflag |= (CLOCAL | CREAD);    // enable receiver, local mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw input, no echo
    tty.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL | INLCR); // no flow control
    tty.c_oflag &= ~(OPOST);            // raw output

    // Set character size to 8 bits:
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    // No parity, one stop bit:
    tty.c_cflag &= ~(PARENB | CSTOPB);

    // apply the new settings:
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        exitWithError("unable to set the /dev/ttyATH0 tty settings");
    }

    printf("Serial port configured to 9600 baud, raw, no echo\n");

    return fd;
}

int openkeyboardEvent(void){
    int fd;
    
    fd = open("/dev/input/event0", O_RDONLY);
    if (fd < 0)
        exitWithError("open /dev/input/event0");
    return fd;
}

int main(int ac, char **av){
    int serial;
    // int displayServer;
    // int kbd;
    
    if (ac != 3)
        return fprintf(stderr, "usage: ./%s [display-ip] [display-port]\n", av[0]);
    
    // displayServer = connectToDisplay(av[0], av[1]);
    serial = openSerialCommunication();

    write(serial,"pinmode 12 output\n", 18);
    while (1) {
        write(serial,"write 12 high\n", 14);
        sleep(1);
        write(serial,"write 12 low\n", 13);
        sleep(1);
    }
    close(serial);
    // kbd = openKeyboardEvent();
}