#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

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
    erow* row = (editor.cursorY >= editor.numRows ? nullptr : &editor.row[editor.cursorY]);

    switch (key) {
        case ARROW_LEFT:
            if (editor.cursorX != 0)
                editor.cursorX--;
            else if (editor.cursorY > 0) {
                editor.cursorY--;
                editor.cursorX = editor.row[editor.cursorY].size;
            }
            break;
        case ARROW_UP:
            if (editor.cursorY != 0)
                editor.cursorY--;
            break;
        case ARROW_DOWN:
            if (editor.cursorY < editor.numRows)
                editor.cursorY++;
            break;
        case ARROW_RIGHT:
            if (row && editor.cursorX < row->size)
                editor.cursorX++;
            else if (row && editor.cursorX >= row->size) {
                editor.cursorY++;
                editor.cursorX = 0;
            }
            break;
        default:
            break;
    }

    row = editor.cursorY >= editor.numRows ? nullptr : &editor.row[editor.cursorY];
    int rowLength = row ? row->size : 0;
    if (editor.cursorX > rowLength) {
        editor.cursorX = rowLength;
    }
}

void editorAppendRow(const char* s, const int len) {
    editor.row = realloc(editor.row, sizeof(erow) * (editor.numRows + 1));
    const int at = editor.numRows;
    editor.row[at].size = len;
    editor.row[at].data = malloc(len+1);
    memcpy(editor.row[at].data, s, len);
    editor.row[at].data[len] = '\0';
    editor.numRows++;
}

void editorOpen(char *filename) {
    FILE* file = fopen(filename, "r");
    if (!file) crash("fopen failed");
    char* line = nullptr;
    size_t lineCap = 0;
    ssize_t lineLen;
    while ((lineLen = getline(&line, &lineCap, file)) != -1) {
        while (lineLen > 0 && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r')) {
            lineLen--;
        }
        editorAppendRow(line, lineLen);
    }
    free(line);
    fclose(file);
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
            editor.cursorX = editor.screenCols - 1;
            break;
        case PAGE_UP:
        case PAGE_DOWN: {
            int times = editor.screenRows;
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
    editorScroll();
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H",3);

    editorDrawRows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (editor.cursorY - editor.rowOffset) + 1, (editor.cursorX - editor.colOffset) + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.buffer, ab.length);
    abFree(&ab);
}

void editorDrawRows(struct abuf *ab) {
    for (int row = 0; row < editor.screenRows; row++) {
        int fileRow = row + editor.rowOffset;
        if (fileRow >= editor.numRows) {
            if (editor.numRows == 0 && row == editor.screenRows / 4) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "ICtext editor - version %s", ICTEXT_VERSION);
                if (welcomelen >= editor.screenCols) {
                    welcomelen = editor.screenCols;
                }
                int padding = (editor.screenCols - welcomelen) / 2;
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
        }
        else {
            int len = editor.row[fileRow].size - editor.colOffset;
            if (len < 0) len = 0;
            if (len > editor.screenCols) { len = editor.screenCols; }
            abAppend(ab, &editor.row[fileRow].data[editor.colOffset], len);
        }
        abAppend(ab, "\x1b[K", 3);
        if (row < editor.screenRows - 1) {
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorScroll() {
    if (editor.cursorY < editor.rowOffset) {
        editor.rowOffset = editor.cursorY;
    }
    if (editor.cursorY >= editor.rowOffset + editor.screenRows) {
        editor.rowOffset = editor.cursorY - editor.screenRows + 1;
    }
    if (editor.cursorX < editor.colOffset) {
        editor.colOffset = editor.cursorX;
    }
    if (editor.cursorX >= editor.colOffset + editor.screenCols) {
        editor.colOffset = editor.cursorX - editor.screenCols + 1;
    }
}

void editorInit() {
    editor.cursorX = 0;
    editor.cursorY = 0;
    editor.numRows = 0;
    editor.rowOffset = 0;
    editor.colOffset = 0;
    editor.row = nullptr;

    if (getWindowSize(&editor.screenRows, &editor.screenCols) == -1) crash("getWindowSize");
}
