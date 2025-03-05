#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "include/editor.h"

#include <stdio.h>
#include <string.h>

#include "include/append_buffer.h"
#include "include/editor_config.h"
#include "include/error_handling.h"

enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DELETE,
    HOME,
    END,
    PAGE_UP,
    PAGE_DOWN,
};

int editorReadKey() {
    ssize_t nread;
    char c = '\0';
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno == EAGAIN) crash("read");
    }
    if (c == '\x1b') {
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1 || read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME;
                        case '3': return DELETE;
                        case '4': return END;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME;
                        case '8': return END;
                        default: break;
                    }
                }
            }
            else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME;
                    case 'F': return END;
                    default: break;
                }
            }
        }
        else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME;
                case 'F': return END;
                default: break;
            }
        }
        return '\x1b';
    }
    return c;
}

void editorMoveCursor(const int key) {
    switch (key) {
        case ARROW_LEFT:
            if (editor.cursorX != 0)
                editor.cursorX--;
            break;
        case ARROW_UP:
            if (editor.cursorY != 0)
                editor.cursorY--;
            break;
        case ARROW_DOWN:
            if (editor.cursorY != editor.rows - 1)
                editor.cursorY++;
            break;
        case ARROW_RIGHT:
            if (editor.cursorX != editor.cols - 1)
                editor.cursorX++;
            break;
        default:
            break;
    }
}

void editorProcessKeypress() {
    const int c = editorReadKey();
    switch (c) {
        case CTRL('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
        case HOME:
            editor.cursorX = 0;
            break;
        case END:
            editor.cursorX = editor.cols - 1;
            break;
        case PAGE_UP:
        case PAGE_DOWN: {
            int times = editor.rows;
            while (times--) {
                editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;
        }
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
        default: break;
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
    editor.cursorX = 0;
    editor.cursorY = 0;

    if (getWindowSize(&editor.rows, &editor.cols) == -1) crash("getWindowSize");
}
