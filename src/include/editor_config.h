#ifndef EDITOR_CONFIG_H
#define EDITOR_CONFIG_H
#include <termios.h>
#include <time.h>

#include "defines.h"

struct editorSyntax {
    char* filetype;
    char** filematch;
    char** kw;
    char** tn;
    char* singlelineCommentStart;
    int flags;
};

static char* C_HL_EXTENSIONS[] = { ".c", ".h", ".cpp", ".hpp", nullptr};
static char* C_HL_KEYWORDS[] = {"switch", "if", "while", "for", "break", "continue", "return", "switch", "case", "else", "struct", "union", "typedef", "static", "enum", "class"};
static char* C_HL_TYPENAMES[] = {"int", "long", "double", "float", "unsigned", "char", "void", "nullptr", "NULL", "true", "false"};

static struct editorSyntax HLDB[] = {{"c", C_HL_EXTENSIONS, C_HL_KEYWORDS, C_HL_TYPENAMES,"//",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS}};

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
