#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include "include/raw_mode.h"
#include "include/error_handling.h"
struct termios original;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &original) == -1) crash("tcgetattr");
    atexit(disableRawMode);

    struct termios raw = original;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ICRNL | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) crash("tcsetattr");
}