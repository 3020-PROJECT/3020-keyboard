#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <linux/input.h>
#include <poll.h>
#include <arpa/inet.h>


/*
    - [X] connect to the esp-displayer
    - [X] identify as 3020-NANO
    - [X] change the baudrate to 9600
    - [X] open /dev/ttyATH0 for both reading and writing.
    - [X] open /dev/input/event0 for reading.
    - [X] open socket with the esp32
    - [X] use poll to not block the IO operations.
    - [X] read from the keyboard and send for both the esp32 and the nano.
    - [X] for the esp32 I need to attach a '/print ' first.
    - [X] everything will be lowercase for now.
*/

#define SERIAL_INDEX    0
#define SOCKET_INDEX    1
#define KEYBOARD_INDEX  2
#define MAX_FDS         3
#define MAX_BUFF_SIZE   512

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

typedef struct buffer
{
    char s[512];
    uint8_t index;
    uint8_t size;
}_buffer;

_buffer serial_buffer;
_buffer display_buffer;

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

    printf("Serial port configured to 9600 baud, raw, no echo; FD= %d\n", fd);

    return fd;
}

int openkeyboardEvent(void){
    int fd;
    
    fd = open("/dev/input/event0", O_RDONLY);
    if (fd < 0)
        exitWithError("open /dev/input/event0");
    return fd;
}

void    appendChar(_buffer *buff, char c){
    if (buff->size > MAX_BUFF_SIZE - 1)
        return ; // can't do more
    buff->s[buff->size] = c;
    buff->s[buff->size + 1] = 0;
    buff->size += 1;
}

void    appendStr(_buffer *buff, char *s){
    while ((buff->size < MAX_BUFF_SIZE - 1) && *s) {
        buff->s[buff->size] = *s;
        s++, buff->size++;
    }
    buff->s[buff->size] = '\0';
}

char    readKey(int fd){
    struct input_event ev;
    if (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
        if (ev.type == EV_KEY && ev.value == 1) {
            char c = keymap[ev.code];
            if (c){
                appendChar(&serial_buffer, c); 
                appendChar(&display_buffer, c);
                return c;
            }
        }
    }
    return -1;
}

void    sendBufferToSocket(int sock_fd){
    int     ret, size;
    char *buff_start;

    buff_start = &display_buffer.s[display_buffer.index];
    if (display_buffer.index == display_buffer.size)
        return ;
    size = strlen(buff_start);

    ret = send(sock_fd, buff_start, size, 0);
    if (ret < 0)
        return ;
    if (buff_start[ret - 1] == '\n'){
        display_buffer.size = 0;
        display_buffer.index = 0;
        appendStr(&display_buffer, "/print ");
    }
    else
        display_buffer.index += ret;
}


void    sendBufferToSerial(int sock_fd){
    int     ret, size;
    char *buff_start;


    buff_start = &serial_buffer.s[serial_buffer.index];
    if (serial_buffer.index == serial_buffer.size)
        return ;
    size = strlen(buff_start);

    ret = write(sock_fd, buff_start, size);
    if (ret < 0)
        return ;
    if (buff_start[ret - 1] == '\n'){
        serial_buffer.size = 0;
        serial_buffer.index = 0;
    }
    else
        serial_buffer.index += ret;
}

void    pollEvents(struct pollfd *fds) {
    int ret = poll(fds, MAX_FDS, -1); // wait indefinitely

    if (ret < 0)
        return perror("POLL FAILED");

    // Check socket events
    if (fds[SOCKET_INDEX].revents & POLLOUT) {
        sendBufferToSocket(fds[SOCKET_INDEX].fd);
    }

    // // Check serial input
    // if (fds[SERIAL_INDEX].revents & POLLIN) {
        
    // }

    // Check serial output ready
    if (fds[SERIAL_INDEX].revents & POLLOUT)
        sendBufferToSerial(fds[SERIAL_INDEX].fd);

    // Check keyboard input
    if (fds[KEYBOARD_INDEX].revents & POLLIN)
        readKey(fds[KEYBOARD_INDEX].fd);
}

int main(int ac, char **av){
    int serial, displayServer, kbd;
    
    if (ac != 3)
        return fprintf(stderr, "usage: ./%s [display-ip] [display-port]\n", av[0]);
    
    displayServer = connectToDisplay(av[1], atoi(av[2]));
    serial = openSerialCommunication();
    kbd = openkeyboardEvent();
    
    struct pollfd fds[MAX_FDS] = {
        [SOCKET_INDEX] = { displayServer,   POLLOUT, 0 },   // socket: only read
        [SERIAL_INDEX] = { serial, POLLIN | POLLOUT, 0 },   // serial: read + write
        [KEYBOARD_INDEX] = { kbd,   POLLIN, 0 }             // control: keyboard input
    };
    
    appendStr(&display_buffer, "/print Hello From 3020-Keyboard!\n/identify 3020-ROUTER\n/print ");
    while (1337)
        pollEvents(fds);
    
    close(serial);
    close(displayServer);
    close(kbd);
}