#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H
#include <termios.h>
struct editorConfig {
    int cursorX, cursorY;
    int rows;
    int cols;
    struct termios original_mode;
};

static struct editorConfig editor;

void disableRawMode();
void enableRawMode();
int getWindowSize(int* rows, int* cols);
#endif // EDITOR_CONFIG_H
