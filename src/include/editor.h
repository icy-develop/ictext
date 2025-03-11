//
// Created by ivan on 03.03.25.
//

#ifndef EDITOR_H
#define EDITOR_H

#include "append_buffer.h"
#include "editor_config.h"

#define ICTEXT_VERSION "0.1.0"
#define TAB_STOP 8

int editorReadKey();
void editorMoveCursor(int key);
void editorOpen(char *filename);
void editorProcessKeypress();
void editorRefreshScreen();
int editorRowCursorXToRenderX(const erow* row, int cursorX);
void editorAppendRow(const char* s, int len);
void editorUpdateRow(erow* row);
void editorDrawRows(struct abuf *ab);
void editorScroll();
void editorInit();

#endif //EDITOR_H
