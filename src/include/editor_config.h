#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H
#include <termios.h>

typedef struct erow {
    int size;
    char* data;
} erow;

struct editorConfig {
    int cursorX, cursorY;
    int screenRows, screenCols;
    int numRows;
    int rowOffset, colOffset;
    erow *row;
    struct termios original_mode;
};

static struct editorConfig editor;

void disableRawMode();
void enableRawMode();
int getWindowSize(int* rows, int* cols);
#endif // EDITOR_CONFIG_H
