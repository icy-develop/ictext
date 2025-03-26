#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H
#include <termios.h>
#include <time.h>

#include "defines.h"

typedef struct editorSyntax {
    char* filetype;
    char** filematch;
    int flags;
} syntax;

static char* C_HL_EXTENSTIONS[] = { ".c", ".h", ".cpp", ".hpp", nullptr};

static syntax HLDB[] = {{"c", C_HL_EXTENSTIONS, HL_HIGHLIGHT_NUMBERS}};

#define HLDB_ENTRIES sizeof(HLDB) / sizeof(HLDB[0])

typedef struct erow {
    int size;
    int rsize;
    char* data;
    char* render;
    unsigned char* hl;
} erow;

struct editorConfig {
    int cursorX, cursorY;
    int renderX;
    int screenRows, screenCols;
    int numRows;
    int rowOffset, colOffset;
    erow *row;
    int dirty;
    char* filename;
    char statusmsg[80];
    time_t statusmsgTime;
    struct editorSyntax* syntax;
    struct termios original_mode;
};

static struct editorConfig editor;



void disableRawMode();
void enableRawMode();
int getWindowSize(int* rows, int* cols);
#endif // EDITOR_CONFIG_H
