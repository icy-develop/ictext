#include "include/error_handling.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void crash(const char* msg) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    perror(msg);
    exit(1);
}
