#include <linux/input.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

/*  Reading Material:
        -> https://www.kernel.org/doc/html/v5.0/input/event-codes.html#device-properties
*/


# define KEY_RELEASE    0
# define KEY_PRESS      1
# define KEY_REPEATED   2

// -------------------- Key mappings --------------------
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
    [KEY_BACKSPACE]="\x7f" // backspace for shell
};

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
    [KEY_BACKSPACE]="\x7f"
};

int main(int ac, char **av){

    struct input_event ev;
    int fd;

    fd = open("/dev/input/event0", O_RDONLY);
    if (fd < 0)
        return 1;

    while (1337) {
        if (read(fd, &ev, sizeof(ev)) == sizeof(ev)){
            if (ev.type == EV_KEY && ev.value != KEY_RELEASE && keymap[ev.code])
                write(1, keymap[ev.code], 1);
        }
    }
}