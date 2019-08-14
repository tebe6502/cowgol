#include <stdarg.h>
#include "globals.h"
#include "emitter.h"

struct chunk
{
    struct chunk* prev;
    size_t fill;
    size_t max;
    char* data;
};

static struct chunk* current = NULL;
static FILE* outfile;

void emitter_open(const char* filename)
{
    outfile = filename ? fopen(filename, "w") : stdout;
    current = NULL;
}

void emitter_close(void)
{
    if (current)
        fatal("output chunks still open");
}

void emitter_open_chunk(void)
{
    struct chunk* newchunk = calloc(1, sizeof(struct chunk));
    newchunk->prev = current;
    newchunk->fill = 0;
    newchunk->max = 256;
    newchunk->data = malloc(256);
    current = newchunk;
}

void emitter_close_chunk(void)
{
    if (!current)
        fatal("no output chunk open");

    fwrite(current->data, 1, current->fill, outfile);

    struct chunk* oldchunk = current;
    current = oldchunk->prev;
    free(oldchunk->data);
    free(oldchunk);
}

void emitter_vprintf(const char* fmt, va_list ap)
{
    if (!current)
        fatal("no output chunk open");

    va_list ap1;
    va_copy(ap1, ap);
    int len = vsnprintf(NULL, 0, fmt, ap1) + 1;
    va_end(ap1);

    while ((current->fill + len) > current->max)
    {
        current->max *= 2;
        current->data = realloc(current->data, current->max);
    }

    current->fill += vsprintf(
        current->data + current->fill,
        fmt, ap);
}

void emitter_printf(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    emitter_vprintf(fmt, ap);
    va_end(ap);
}
