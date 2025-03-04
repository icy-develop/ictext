//
// Created by ivan on 04.03.25.
//

#ifndef APPEND_BUFFER_H
#define APPEND_BUFFER_H

struct abuf {
    char *buffer;
    int length;
};
#define ABUF_INIT { NULL, 0 }

void abAppend(struct abuf *ab, const char *str, unsigned len);
void abFree(const struct abuf *ab);

#endif //APPEND_BUFFER_H
