#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "include/editor_config.h"

#include <ctype.h>
#include <stdio.h>

#include "include/editor.h"
#include "include/error_handling.h"



void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.original_mode);
}
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &editor.original_mode) == -1) crash("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = editor.original_mode;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ICRNL | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) crash("tcsetattr");
}

int getCursorPosition(int* rows, int* cols) {
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    char buf[32];
    uint i;
    for (i = 0; i < sizeof(buf) - 1; i++) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d",rows, cols) != 2) return -1;

    return 0;
}

int getWindowSize(int* rows, int* cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    }
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}
