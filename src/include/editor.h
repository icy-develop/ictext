//
// Created by ivan on 03.03.25.
//

#ifndef EDITOR_H
#define EDITOR_H

#include "append_buffer.h"

#define ICTEXT_VERSION "0.1.0"

char editorReadKey();
void editorMoveCursor(char key);
void editorProcessKeypress();
void editorRefreshScreen();
void editorDrawRows(struct abuf *ab);
void editorInit();

#endif //EDITOR_H
