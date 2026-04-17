#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include <stdio.h>

/*
    Using arena allocator so that we can more easily manage
    memory later.  Just using malloc (and free does nothing)
    for now.
*/
struct arena {
    int placeholder;
};

void arena_init(struct arena *arena) {
    //silence warning
    arena = arena;
    //TODO initialize custom allocator
}

void* arena_malloc(struct arena *arena, size_t size) {
    //silence warning
    arena = arena;
    void *p = malloc(size); //TODO use custom allocation later
    if (!p) {
	    fprintf(stderr,"malloc failed\n");
        exit(1);
    }
    return p;
}

void arena_free(struct arena *arena) {
    //silence warning
    arena = arena;
    //TODO should free all memory in arena later
}


#endif //ARENA_H
