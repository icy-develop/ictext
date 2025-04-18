//
// Created by ivan on 03.03.25.
//

#ifndef EDITOR_H
#define EDITOR_H

#include "append_buffer.h"
#include "editor_config.h"

#define ICTEXT_VERSION "0.1.0"
#define TAB_STOP 8
#define STATUS_MSG_TIMEOUT 5
#define QUIT_MSG_RETRY 2

int editorReadKey();
void editorMoveCursor(int key);
void editorOpen(char *filename);
void editorProcessKeypress();
void editorRefreshScreen();
char* editorPrompt(const char *prompt, void(*callback)(const char*, int));

int isSeparator(int c);
void editorSelectSyntaxHighlight();
void editorUpdateSyntax(erow* row);
int editorSyntaxToColor(int hl);

void editorFind();
void editorFindCallback(const char* query, int key);

void editorSetStatusMessage(const char *format, ...);
int editorRowCursorXToRenderX(const erow* row, int cursorX);
int editorRenderXToCursorX(const erow* row, int renderX);

char* editorRowsToString(int* bufferLength);
void editorRowInsertChar(erow* row, int at, int c);
void editorRowDeleteChar(erow* row, int at);
void editorRowAppendString(erow* row, char* str, size_t length);
void editorInsertChar(int c);
void editorDeleteChar();

void editorInsertNewLine();
void editorInsertRow(int at, const char* s, int len);
void editorUpdateRow(erow* row);
void editorDeleteRow(int at);
void editorFreeRow(erow* row);
void editorDrawRows(struct abuf *ab);

void editorDrawStatusBar(struct abuf *ab);
void editorDrawStatusMessage(struct abuf *ab);

void editorSave();
void editorScroll();
void editorInit();

#endif //EDITOR_H
