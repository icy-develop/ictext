#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "include/editor.h"
#include "include/editor_config.h"
#include "include/error_handling.h"

char editorReadKey() {
    ssize_t nread;
    char c = '\0';
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno == EAGAIN) crash("read");
    }
    return c;
}

void editorProcessKeypress() {
    const char c = editorReadKey();
    switch (c) {
        case CTRL('q'):
            editorRefreshScreen();
            exit(0);
        default:
            break;
    }
}

void editorRefreshScreen() {
    write(STDIN_FILENO, "\x1b[2J",4);
    write(STDIN_FILENO, "\x1b[H",3);

    editorDrawRows();

    write(STDIN_FILENO, "\x1b[H",3);
}

void editorDrawRows() {
    for (int row = 0; row < editor.rows; row++) {
        write(STDOUT_FILENO, "~", 1);
        if (row < editor.rows - 1) {
            write(STDOUT_FILENO, "\r\n", 2);
        }
    }
}

void editorInit() {
    if (getWindowSize(&editor.rows, &editor.cols) == -1) crash("getWindowSize");
}
