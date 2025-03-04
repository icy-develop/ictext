#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "include/editor.h"

#include <stdio.h>
#include <string.h>

#include "include/append_buffer.h"
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

void editorMoveCursor(const char key) {
    switch (key) {
        case 'a':
            editor.cursorX--;
            break;
        case 'w':
            editor.cursorY--;
            break;
        case 's':
            editor.cursorY++;
            break;
        case 'd':
            editor.cursorX++;
            break;
        default:
            break;
    }
}

void editorProcessKeypress() {
    const char c = editorReadKey();
    switch (c) {
        case CTRL('q'):
            editorRefreshScreen();
            exit(0);
        case 'a':
        case 'd':
        case 's':
        case 'w':
            editorMoveCursor(c);
            break;
    }
}

void editorRefreshScreen() {
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H",3);

    editorDrawRows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.cursorY + 1, editor.cursorX + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.buffer, ab.length);
    abFree(&ab);
}

void editorDrawRows(struct abuf *ab) {
    for (int row = 0; row < editor.rows; row++) {
        if (row == editor.rows / 4) {
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome), "ICtext editor - version %s", ICTEXT_VERSION);
            if (welcomelen >= editor.cols) {
                welcomelen = editor.cols;
            }
            int padding = (editor.cols - welcomelen) / 2;
            if (padding) {
                abAppend(ab, "~", 1);
                padding--;
            }
            while (padding--) {
                abAppend(ab, " ", 1);
            }
            abAppend(ab, welcome, welcomelen);
        }
        else
            abAppend(ab, "~", 1);
        abAppend(ab, "\x1b[K", 3);
        if (row < editor.rows - 1) {
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorInit() {
    editor.cursorX = 20;
    editor.cursorY = 20;

    if (getWindowSize(&editor.rows, &editor.cols) == -1) crash("getWindowSize");
}
