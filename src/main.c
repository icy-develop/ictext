#include "include/editor.h"
#include "include/editor_config.h"

int main(int argc, char **argv) {
    enableRawMode();
    editorInit();
    if (argc >= 2) {
        editorOpen(argv[1]);
    }

    editorSetStatusMessage("HELP: Ctrl-Q = quit");
    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}