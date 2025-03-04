//
// Created by ivan on 04.03.25.
//

#include "include/append_buffer.h"

#include <stdlib.h>
#include <string.h>

#include "include/editor_config.h"

void abAppend(struct abuf *ab, const char *str, const unsigned len) {
    char* new = realloc(ab->buffer, ab->length + len);

    if (new == nullptr) return;
    memcpy(&new[ab->length], str, len);
    ab->buffer = new;
    ab->length += len;
}

void abFree(const struct abuf *ab) {
    free(ab->buffer);
}