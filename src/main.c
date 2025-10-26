#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <linux/input.h>
#include <arpa/inet.h>


/*
    - [X] connect to the esp-displayer
    - [X] identify as 3020-NANO
    - [X] change the baudrate to 9600
    - [X] open /dev/ttyATH0 for both reading and writing.
    - [X] open /dev/input/event0 for reading.
    - [X] open socket with the esp32
    - [] use poll to not block the IO operations.
    - [] read from the keyboard and send for both the esp32 and the nano.
    - [X] for the esp32 I need to attach a '/print ' first.
    - [] everything will be lowercase for now.
*/

const char keymap[256] = {
    [KEY_A]='a', [KEY_B]='b', [KEY_C]='c', [KEY_D]='d',
    [KEY_E]='e', [KEY_F]='f', [KEY_G]='g', [KEY_H]='h',
    [KEY_I]='i', [KEY_J]='j', [KEY_K]='k', [KEY_L]='l',
    [KEY_M]='m', [KEY_N]='n', [KEY_O]='o', [KEY_P]='p',
    [KEY_Q]='q', [KEY_R]='r', [KEY_S]='s', [KEY_T]='t',
    [KEY_U]='u', [KEY_V]='v', [KEY_W]='w', [KEY_X]='x',
    [KEY_Y]='y', [KEY_Z]='z',
    [KEY_1]='1', [KEY_2]='2', [KEY_3]='3', [KEY_4]='4',
    [KEY_5]='5', [KEY_6]='6', [KEY_7]='7', [KEY_8]='8',
    [KEY_9]='9', [KEY_0]='0',
    [KEY_ENTER]='\n', [KEY_SPACE]=' ',
    [KEY_DOT]='.', [KEY_COMMA]=',',
    [KEY_MINUS]='-', [KEY_EQUAL]='=',
    [KEY_BACKSPACE]='\b'
};

void    exitWithError(char *error){
    perror(error);
    exit(1);
}

int connectToDisplay(char *display_ip, int  display_port){
    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        exitWithError("Socket");
     memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(display_port);
    if (inet_pton(AF_INET, display_ip, &serv_addr.sin_addr) <= 0){
        close(sock);
        exitWithError("inet_pton");
    }
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        close(sock);
        exitWithError("connect");
    }

    write(sock, "/print Hello Test\n", 18);
    write(sock, "/identify 3020-ROUTER\n", 22);
    return sock;
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

char    readKey(int fd){
    struct input_event ev;
    if (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY && ev.value == 1) {
            char c = keymap[ev.code];
            return c;
        }
    }
    return -1;
}

int main(int ac, char **av){
    int serial;
    int displayServer;
    int kbd;
    
    if (ac != 3)
        return fprintf(stderr, "usage: ./%s [display-ip] [display-port]\n", av[0]);
    
    displayServer = connectToDisplay(av[1], atoi(av[2]));
    serial = openSerialCommunication();
    kbd = openkeyboardEvent();

    write(displayServer, "/print ", 7);
    while (1337) {
        char c = readKey(kbd);
        if (c > 0){
            write(1, &c, 1);
            write(serial, &c, 1);
            write(displayServer, &c, 1);
            if (c == '\n')
                write(displayServer, "/print ", 7);
        }
    }
    close(serial);
    close(displayServer);
    close(kbd);
}