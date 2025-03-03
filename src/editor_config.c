#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "include/editor_config.h"
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

int getWindowSize(int* rows, int* cols) {
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    }
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}
