#include "include/editor.h"
#include "include/editor_config.h"

int main(void) {
    enableRawMode();
    editorInit();
    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}