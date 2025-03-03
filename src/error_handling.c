#include "include/error_handling.h"

#include <stdio.h>
#include <stdlib.h>

void crash(const char* msg) {
    perror(msg);
    exit(1);
}
