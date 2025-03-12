#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "include/editor.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#include "include/append_buffer.h"
#include "include/editor_config.h"
#include "include/error_handling.h"

enum editorKey {
    BACKSPACE = 127,
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
            } else {
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
        } else if (seq[0] == 'O') {
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
    erow *row = (editor.cursorY >= editor.numRows ? nullptr : &editor.row[editor.cursorY]);

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

void editorAppendRow(const char *s, const int len) {
    editor.row = realloc(editor.row, sizeof(erow) * (editor.numRows + 1));
    const int at = editor.numRows;
    editor.row[at].size = len;
    editor.row[at].data = malloc(len + 1);
    memcpy(editor.row[at].data, s, len);
    editor.row[at].data[len] = '\0';

    editor.row[at].rsize = 0;
    editor.row[at].render = nullptr;
    editorUpdateRow(&editor.row[at]);

    editor.numRows++;
    editor.dirty++;
}

void editorUpdateRow(erow *row) {
    int tabs = 0;
    for (int i = 0; i < row->size; i++) {
        if (row->data[i] == '\t') tabs++;
    }
    free(row->render);
    row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

    int idx = 0;
    for (int j = 0; j < row->size; j++) {
        if (row->data[j] == '\t') {
            row->render[idx++] = ' ';
            while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->data[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}

void editorDeleteRow(int at) {
    if (at < 0 || at >= editor.numRows) return;
    editorFreeRow(&editor.row[at]);
    memmove(&editor.row[at], &editor.row[at + 1], (editor.numRows - at - 1) * sizeof(erow));
    editor.numRows--;
    editor.dirty++;
}

void editorFreeRow(erow *row) {
    free(row->render);
    free(row->data);
}

void editorOpen(char *filename) {
    free(editor.filename);
    editor.filename = strdup(filename);
    FILE *file = fopen(filename, "r");
    if (!file) crash("fopen failed");
    char *line = nullptr;
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
    editor.dirty = 0;
}

void editorProcessKeypress() {
    static int quit_times = QUIT_MSG_RETRY;

    const int c = editorReadKey();
    switch (c) {
        case CTRL('q'):
            if (editor.dirty && quit_times > 0) {
                editorSetStatusMessage("File has unsaved changes. Press Ctrl-Q %d more times to exit", quit_times);
                quit_times--;
                return;
            }
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
        case CTRL('s'):
            editorSave();
            break;
        case BACKSPACE:
        case CTRL('h'):
        case DELETE:
            if (c == DELETE) editorMoveCursor(ARROW_RIGHT);
            editorDeleteChar();
            break;
        case CTRL('l'):
        case '\x1b':
            break;
        case '\r':
            break;
        case HOME:
            editor.cursorX = 0;
            break;
        case END:
            if (editor.cursorY < editor.numRows)
                editor.cursorX = editor.row[editor.cursorY].size;
            break;
        case PAGE_UP:
        case PAGE_DOWN: {
            if (c == PAGE_UP) {
                editor.cursorY = editor.rowOffset;
            } else {
                editor.cursorY = editor.rowOffset + editor.screenRows - 1;
                if (editor.cursorY > editor.numRows) editor.cursorY = editor.numRows;
            }
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
        default:
            editorInsertChar(c);
            break;
    }
    quit_times = QUIT_MSG_RETRY;
}

void editorRefreshScreen() {
    editorScroll();
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6);
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawStatusMessage(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (editor.cursorY - editor.rowOffset) + 1,
             (editor.renderX - editor.colOffset) + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.buffer, ab.length);
    abFree(&ab);
}

void editorSetStatusMessage(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(editor.statusmsg, sizeof(editor.statusmsg), format, args);
    va_end(args);
    editor.statusmsgTime = time(nullptr);
}

int editorRowCursorXToRenderX(const erow *row, const int cursorX) {
    int renderX = 0;
    for (int i = 0; i < cursorX; i++) {
        if (row->data[i] == '\t') {
            renderX += (TAB_STOP - 1) - (renderX % TAB_STOP);
        }
        renderX++;
    }
    return renderX;
}

char * editorRowsToString(int *bufferLength) {
    int totalLength = 0;
    for (int i = 0; i < editor.numRows; i++) {
        totalLength += editor.row[i].size + 1;
    }
    *bufferLength = totalLength;

    char* buf = malloc(totalLength);
    char *it = buf;
    for (int i = 0; i < editor.numRows; i++) {
        memcpy(it, editor.row[i].data, editor.row[i].size);
        it += editor.row[i].size;
        *it = '\n';
        it++;
    }
    return buf;
}

void editorRowInsertChar(erow *row, int at, const int c) {
    if (at < 0 || at >= row->size) at = row->size;
    row->data = realloc(row->data, row->size + row->size + 2);
    memmove(&row->data[at + 1], &row->data[at], row->size - at + 1);
    row->size++;
    row->data[at] = c;
    editorUpdateRow(row);
    editor.dirty++;
}

void editorRowDeleteChar(erow *row, int at) {
    if (at < 0 || at >= row->size) return;
    memmove(&row->data[at], &row->data[at + 1], row->size - at);
    row->size--;
    editorUpdateRow(row);
    editor.dirty++;
}

void editorRowAppendString(erow *row, char *str, size_t length) {
    row->data = realloc(row->data, row->size + length + 1);
    memcpy(&row->data[row->size], str, length);
    row->size += length;
    row->data[row->size] = '\0';
    editorUpdateRow(row);
    editor.dirty++;
}

void editorInsertChar(int c) {
    if (editor.cursorY == editor.numRows) {
        editorAppendRow("", 0);
    }
    editorRowInsertChar(&editor.row[editor.cursorY], editor.cursorX, c);
    editor.cursorX++;
}


void editorDeleteChar() {
    if (editor.cursorY == editor.numRows) return;
    if (editor.cursorX == 0 && editor.cursorY == 0) return;

    erow* row = &editor.row[editor.cursorY];
    if (editor.cursorX > 0) {
        editorRowDeleteChar(row, editor.cursorX - 1);
        editor.cursorX--;
    }
    else {
        editor.cursorX = editor.row[editor.cursorY - 1].size;
        editorRowAppendString(&editor.row[editor.cursorY - 1], row->data, row->size);
        editorDeleteRow(editor.cursorY);
        editor.cursorY--;
    }
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
            } else
                abAppend(ab, "~", 1);
        } else {
            int len = editor.row[fileRow].rsize - editor.colOffset;
            if (len < 0) len = 0;
            if (len > editor.screenCols) { len = editor.screenCols; }
            abAppend(ab, &editor.row[fileRow].render[editor.colOffset], len);
        }
        abAppend(ab, "\x1b[K", 3);
        abAppend(ab, "\r\n", 2);
    }
}

void editorDrawStatusBar(struct abuf *ab) {
    abAppend(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int length = snprintf(status, sizeof(status), "%.20s - %d lines %s", editor.filename ? editor.filename : "[Blank]",
                          editor.numRows, editor.dirty ? "(modified)" : "");
    int rLength = snprintf(rstatus, sizeof(rstatus), "%d/%d", editor.cursorY + 1, editor.numRows);
    if (length > editor.screenCols) length = editor.screenCols;
    abAppend(ab, status, length);
    while (length < editor.screenCols) {
        if (editor.screenCols - length == rLength) {
            abAppend(ab, rstatus, rLength);
            break;
        }

        abAppend(ab, " ", 1);
        length++;
    }
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

void editorDrawStatusMessage(struct abuf *ab) {
    abAppend(ab, "\x1b[K", 3);
    u_long msgLength = strlen(editor.statusmsg);
    if (msgLength > editor.screenCols) msgLength = editor.screenCols;
    if (msgLength && time(nullptr) - editor.statusmsgTime < STATUS_MSG_TIMEOUT)
        abAppend(ab, editor.statusmsg, msgLength);
}

void editorScroll() {
    editor.renderX = 0;
    if (editor.cursorY < editor.numRows) {
        editor.renderX = editorRowCursorXToRenderX(&editor.row[editor.cursorY], editor.cursorX);
    }
    if (editor.cursorY < editor.rowOffset) {
        editor.rowOffset = editor.cursorY;
    }
    if (editor.cursorY >= editor.rowOffset + editor.screenRows) {
        editor.rowOffset = editor.cursorY - editor.screenRows + 1;
    }
    if (editor.renderX < editor.colOffset) {
        editor.colOffset = editor.renderX;
    }
    if (editor.renderX >= editor.colOffset + editor.screenCols) {
        editor.colOffset = editor.renderX - editor.screenCols + 1;
    }
}

void editorSave() {
    if (editor.filename == nullptr) return;

    int length;
    char* buffer = editorRowsToString(&length);

    const int fd = open(editor.filename, O_RDWR | O_CREAT, 0644);
    if (fd != -1) {
        if (ftruncate(fd, length) != -1) {
            if (write(fd, buffer, length) == length) {
                close(fd);
                free(buffer);
                editor.dirty = 0;
                editorSetStatusMessage("%d bytes saved on disk", length);
                return;
            }
        }
        close(fd);
    }
    free(buffer);
    editorSetStatusMessage("Can't save! I/O Error: %s", strerror(errno));
}

void editorInit() {
    editor.cursorX = 0;
    editor.cursorY = 0;
    editor.renderX = 0;
    editor.numRows = 0;
    editor.rowOffset = 0;
    editor.colOffset = 0;
    editor.row = nullptr;
    editor.dirty = 0;
    editor.filename = nullptr;
    editor.statusmsg[0] = '\0';
    editor.statusmsgTime = 0;
    if (getWindowSize(&editor.screenRows, &editor.screenCols) == -1) crash("getWindowSize");
    editor.screenRows -= 2;
}
