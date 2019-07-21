#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "global_vars.h"

#define DEFAULT_OUTBUF_SIZE (1024)

void
ruc_context_init(ruc_context *context)
{
    memset(context, 0, sizeof(ruc_context));
    scanner_init(&context->input_options);
    printer_init(&context->output_options);
    printer_init(&context->err_options);
    printer_init(&context->miscout_options);
    context->charnum = 1;
    context->m_charnum = 1;
    context->rp = 1;
    context->id = 2;
    context->md = 1;
    context->startmode = 1;
    context->sopnd = -1;
    context->curid = 2;
    context->lg = -1;
    context->displ = -3;
    context->maxdispl = 3;
    context->maxdisplg = 3;
    context->procd = 1;
    context->pc = 4;
    context->funcnum = 2;
    context->blockflag = 1;
    context->notrobot = 1;
    context->prdf = -1;
    context->leftansttype = -1;

    context->fip = 1;
    context->mfp = 1;
    context->mfirstrp = -1;
    context->mlastrp = -1;
    context->mp = 3;
}

/** Is I/O an actual output? */
static bool
io_type_is_output(ruc_io_type type)
{
    switch (type)
    {
        case IO_TYPE_INPUT:
            return false;
        default:
            return true;
    }
}

/** Get type-specific options */
static void *
io_type2opts(ruc_context *context, ruc_io_type type)
{
    switch (type)
    {
        case IO_TYPE_INPUT:
            return &context->input_options;
        case IO_TYPE_OUTPUT:
            return &context->output_options;
        case IO_TYPE_ERROR:
            return &context->err_options;
        case IO_TYPE_MISC:
            return &context->miscout_options;
        default:
            return NULL;
    }
}

/**
 * Get FILE for a specific IO type
 */
static FILE **
io_type2file(ruc_context *context, ruc_io_type type)
{
    switch (type)
    {
        case IO_TYPE_INPUT:
            return &context->input_options.input;
        case IO_TYPE_OUTPUT:
            return &context->output_options.output;
        case IO_TYPE_ERROR:
            return &context->err_options.output;
        default:
            return NULL;
    }
}

/**
 * Get access mask for a specific IO type
 */
static const char *
io_type2access_mask(ruc_io_type type)
{
    return io_type_is_output(type) ? "wt" : "r";
}

/**
 * Open file with specific mask taking standard files into account
 */
static FILE *
io_get_file(const char *ptr, const char *mask)
{
    if (strcmp(ptr, ":stderr") == 0)
        return stderr;
    else if (strcmp(ptr, ":stdout") == 0)
        return stdout;
    else if (strcmp(ptr, ":stdin") == 0)
        return stdin;
    return fopen(ptr, mask);
}

/* See description in context.h */
void
ruc_context_attach_io(ruc_context *context, const char *ptr, ruc_io_type type,
                      ruc_io_source source)
{
    void *opts = io_type2opts(context, type);

    if (source == IO_SOURCE_FILE)
    {
        FILE * f = io_get_file(ptr, io_type2access_mask(type));

        if (f == NULL)
        {
            if (io_type_is_output(type))
            {
                printf(" ошибка открытия файла %s: %s\n", ptr,
                       strerror(errno));
            }
            else
            {
                printf(" не найден файл %s\n", ptr);
            }
            exit(1);
        }

        if (io_type_is_output(type))
            printer_attach_file(opts, f);
        else
            scanner_attach_file(opts, f);
    }
    else
    {
        if (io_type_is_output(type))
            printer_attach_buffer(opts, DEFAULT_OUTBUF_SIZE);
        else
            scanner_attach_buffer(opts, ptr);
    }
}

/* See description in context.h */
void
ruc_context_detach_io(ruc_context *context, ruc_io_type type)
{
    if (type == IO_TYPE_OUTPUT ||
        context->input_options.source == IO_SOURCE_FILE)
    {
        FILE **f = io_type2file(context, type);

        if (*f != NULL)
            fclose(*f);
        *f = NULL;
    }
    else
    {
        context->input_options.ptr = NULL;
        context->input_options.pos = -1;
    }
}
