//
// Copyright 2020 Serge Martin
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
#ifndef U_PRINTF_H
#define U_PRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include "simple_mtx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct u_printf_info {
   unsigned num_args;
   unsigned *arg_sizes;
   unsigned string_size;
   char *strings;
} u_printf_info;

struct blob;
struct blob_reader;

void u_printf_serialize_info(struct blob *blob,
                             const u_printf_info *info,
                             unsigned printf_info_count);

u_printf_info *u_printf_deserialize_info(void *mem_ctx,
                                         struct blob_reader *blob,
                                         unsigned *printf_info_count);

uint32_t u_printf_hash(const u_printf_info *info);

void u_printf_singleton_init_or_ref(void);
void u_printf_singleton_decref(void);
void u_printf_singleton_add(const u_printf_info *info, unsigned count);
void u_printf_singleton_add_serialized(const void *data, size_t data_size);
const u_printf_info *u_printf_singleton_search(uint32_t hash);

struct u_printf_ctx {
   simple_mtx_t lock;
   void *bo;
   uint32_t *map;
};

const char *util_printf_prev_tok(const char *str);

/* find next valid printf specifier in a C string wrapper */
size_t util_printf_next_spec_pos(const char *str, size_t pos);

/* Return the length of the string that would be generated by a printf-style
 * format and argument list, not including the \0 byte.
 * The untouched_args parameter is left untouched so it can be re-used by the
 * caller in a vsnprintf() call or similar.
 */
size_t u_printf_length(const char *fmt, va_list untouched_args);

void u_printf(FILE *out, const char *buffer, size_t buffer_size,
              const u_printf_info*, unsigned info_size);

void u_printf_ptr(FILE *out, const char *buffer, size_t buffer_size,
                  const u_printf_info **info, unsigned info_size);

static inline void
u_printf_init(struct u_printf_ctx *ctx, void *bo, uint32_t *map)
{
   ctx->bo = bo;
   ctx->map = map;
   simple_mtx_init(&ctx->lock, mtx_plain);

   /* Initialize the buffer head to point to just after the size + abort word */
   map[0] = 8;

   /* Initially there is no abort */
   map[1] = 0;
}

static inline void
u_printf_destroy(struct u_printf_ctx *ctx)
{
   simple_mtx_destroy(&ctx->lock);
}

static inline void
u_printf_with_ctx(FILE *out, struct u_printf_ctx *ctx)
{
   /* If the printf buffer is empty, early-exit without taking the lock. The
    * speeds up the happy path and makes this function reasonable to call even
    * in release builds.
    */
   if (ctx->map[0] == 8)
      return;

   simple_mtx_lock(&ctx->lock);
   u_printf(out, (char *)(ctx->map + 2), ctx->map[0] - 8, NULL, 0);

   /* Reset */
   ctx->map[0] = 8;
   simple_mtx_unlock(&ctx->lock);
}

/*
 * Flush the printf buffer and return whether there was an abort. This is
 * intended to be called periodically to handle aborts in a timely manner.
 */
static inline bool
u_printf_check_abort(FILE *out, struct u_printf_ctx *ctx)
{
   u_printf_with_ctx(out, ctx);

   /* Check the aborted flag */
   return (ctx->map[1] != 0);
}

#ifdef __cplusplus
}
#endif

#endif
