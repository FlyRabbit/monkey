/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Server
 *  ==================
 *  Copyright 2001-2015 Monkey Software LLC <eduardo@monkey.io>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef MK_UTILS_H
#define MK_UTILS_H

#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mk_macros.h"

#define MK_UTILS_INT2MKP_BUFFER_LEN 16    /* Maximum buffer length when
                                           * converting an int to mk_ptr_t */
/*
 * Max amount of pid digits. Glibc's pid_t is implemented as a signed
 * 32bit integer, for both 32 and 64bit systems - max value: 2147483648.
 */
#define MK_MAX_PID_LEN 10

#include "mk_memory.h"
#include "mk_list.h"

#define MK_GMT_CACHES 10

struct mk_gmt_cache {
    time_t time;
    char text[32];
    unsigned long long hits;
};

/* Trace definitions */
#ifdef TRACE

#define MK_TRACE_CORE 0
#define MK_TRACE_PLUGIN 1
#define MK_TRACE_COMP_CORE "core"

#define MK_TRACE(...) mk_utils_trace(MK_TRACE_COMP_CORE, MK_TRACE_CORE, \
                                     __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
char *env_trace_filter;
pthread_mutex_t mutex_trace;

#else
#define MK_TRACE(...) do {} while (0)
#endif

int    mk_utils_utime2gmt(char **data, time_t date);
time_t mk_utils_gmt2utime(char *date);

int mk_buffer_cat(mk_ptr_t * p, char *buf1, int len1, char *buf2, int len2);

int mk_utils_set_daemon(void);
char *mk_utils_url_decode(mk_ptr_t req_uri);

#ifdef TRACE
void mk_utils_trace(const char *component, int color, const char *function,
                    char *file, int line, const char* format, ...);
int mk_utils_print_errno(int n);
#endif

int mk_utils_register_pid(void);
int mk_utils_remove_pid(void);

void mk_print(int type, const char *format, ...) PRINTF_WARNINGS(2,3);

pthread_t mk_utils_worker_spawn(void (*func) (void *), void *arg);
int mk_utils_worker_rename(const char *title);
void mk_utils_stacktrace(void);

unsigned int mk_utils_gen_hash(const void *key, int len);

/* Thread key to hold a re-entrant buffer for strerror formatting */
#define MK_UTILS_ERROR_SIZE          128
pthread_key_t mk_utils_error_key;

/*
 * Helpers to format and print out common errno errors, we use thread
 * keys to hold a buffer per thread so strerror_r(2) can be used without
 * a memory allocation.
 */
#define MK_UTILS_LIBC_ERRNO_BUFFER()                                  \
    int _err  = errno;                                                \
    char *buf = pthread_getspecific(mk_utils_error_key);              \
    if (strerror_r(_err, buf, MK_UTILS_ERROR_SIZE) != 0) {            \
        mk_err("strerror_r() failed");                                \
    }

static inline void mk_utils_libc_error(char *caller, char *file, int line)
{
    MK_UTILS_LIBC_ERRNO_BUFFER();
    mk_err("%s: %s, errno=%i at %s:%i", caller, buf, _err, file, line);
}

static inline void mk_utils_libc_warning(char *caller, char *file, int line)
{
    MK_UTILS_LIBC_ERRNO_BUFFER();
    mk_warn("%s: %s, errno=%i at %s:%i", caller, buf, _err, file, line);
}

#endif
