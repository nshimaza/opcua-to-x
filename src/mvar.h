#ifndef MVAR_H
#define MVAR_H
/*
 * MIT License
 *
 * Copyright (c) 2020 Naoto Shimazaki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * MVar is one element only thread safe queue.
 * This is MVar implementation in C and pthread.
 */

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

typedef void (*read_callback)(void* const out_user_data, void* const mvar_context);
typedef void (*write_callback)(void* const mvar_context, const void* const user_data);

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t put_cond;
    pthread_cond_t take_cond;
    bool empty;
    read_callback read;
    write_callback write;
} mvar_abs_t;

void init_mvar(void* const out_mvar, read_callback read, write_callback write);
// Initialize MVar with no context.  Emulates MVar ().
void init_mvar_unit(mvar_abs_t* const out_mvar);
bool is_empty_mvar(const void* const mvar);
void put_mvar(void* const mvar, const void* const user_data);
void read_mvar(void* const out_user_data, void* const mvar);
void take_mvar(void* const out_user_data, void* const mvar);
int timed_put_mvar(void* const mvar, const long int timeout_in_msec, const void* const user_data);
int timed_read_mvar(void* const out_user_data, void* const mvar, const long int timeout_in_msec);
int timed_take_mvar(void* const out_user_data, void* const mvar, const long int timeout_in_msec);
int try_put_mvar(void* mvar, const void* const user_data);
int try_read_mvar(void* const out_user_data, void* const mvar);
int try_take_mvar(void* const out_user_data, void* const mvar);

#endif
