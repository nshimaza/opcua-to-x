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

#include <assert.h>
#include <errno.h>
#include <time.h>

#include "mvar.h"
#include "hexdump.h"

void
init_mvar(void* const out_mvar, read_callback read, write_callback write) {
    assert(out_mvar != NULL);
    mvar_abs_t* const v = out_mvar;
    pthread_mutex_init(&v->lock, NULL);
    pthread_cond_init(&v->put_cond, NULL);
    pthread_cond_init(&v->take_cond, NULL);
    v->empty = true;
    assert(read != NULL);
    v->read = read;
    assert(write != NULL);
    v->write = write;
}

void
mvar_abs_write(void* const mvar_context, const void* const user_data) {
    assert(user_data == NULL);
}

void
mvar_abs_read(void* const out_user_data, void* const mvar_context) {
    assert(out_user_data == NULL);
}

void
init_mvar_unit(mvar_abs_t* const out_mvar) {
    init_mvar(out_mvar, mvar_abs_read, mvar_abs_write);
}

bool
isEmpty_mvar(const void* const mvar) {
    assert(mvar != NULL);
    return ((const mvar_abs_t*) mvar)->empty;
}

void
put_mvar(void* const mvar, const void* const user_data) {
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    int err = pthread_mutex_lock(&v->lock);
    assert(err == 0);
    if (!v->empty) {
        pthread_cond_wait(&v->put_cond, &v->lock);
    }
    assert(v->write != NULL);
    v->write(v, user_data);
    v->empty = false;
    pthread_cond_signal(&v->take_cond);
    pthread_mutex_unlock(&v->lock);
}

void
read_mvar(void* const out_user_data, void* const mvar) {
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    int err = pthread_mutex_lock(&v->lock);
    assert(err == 0);
    if (v->empty) {
        pthread_cond_wait(&v->take_cond, &v->lock);
    }
    assert(v->read != NULL);
    v->read(out_user_data, v);
    pthread_mutex_unlock(&v->lock);
}

void
take_mvar(void* const out_user_data, void* const mvar) {
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    int err = pthread_mutex_lock(&v->lock);
    assert(err == 0);
    if (v->empty) {
        pthread_cond_wait(&v->take_cond, &v->lock);
    }
    assert(v->read != NULL);
    v->read(out_user_data, v);
    v->empty = true;
    pthread_cond_signal(&v->put_cond);
    pthread_mutex_unlock(&v->lock);
}

struct timespec
timespec_add(struct timespec already_normalized_timesec, long int nano_second) {
    already_normalized_timesec.tv_nsec += nano_second;
    already_normalized_timesec.tv_sec += already_normalized_timesec.tv_nsec / 1000000000;
    already_normalized_timesec.tv_nsec %= 1000000000;
    return already_normalized_timesec;
}

int
timed_put_mvar(void* const mvar, const long int timeout_in_msec, const void* const user_data) {
    struct timespec now;
    int err = timespec_get(&now, TIME_UTC);
    assert(err == 0);
    struct timespec tout = timespec_add(now, timeout_in_msec * 1000000);
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    err = pthread_mutex_timedlock(&v->lock, &tout);
    if (err != 0) {
        // When timer expired, ETIMEDOUT is returned.
        return err;
    }
    if (!v->empty) {
        err = pthread_cond_timedwait(&v->put_cond, &v->lock, &tout);
        if (!v->empty) {
            pthread_mutex_unlock(&v->lock);
            return err;
        }
    }
    assert(v->write != NULL);
    v->write(v, user_data);
    v->empty = false;
    pthread_cond_signal(&v->take_cond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
timed_read_mvar(void* const out_user_data, void* const mvar, const long int timeout_in_msec) {
    struct timespec now;
    int err = timespec_get(&now, TIME_UTC);
    assert(err == 0);
    struct timespec tout = timespec_add(now, timeout_in_msec * 1000000);
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    err = pthread_mutex_timedlock(&v->lock, &tout);
    if (err != 0) {
        // When timer expired, ETIMEDOUT is returned.
        return err;
    }
    if (v->empty) {
        err = pthread_cond_timedwait(&v->take_cond, &v->lock, &tout);
        if (v->empty) {
            pthread_mutex_unlock(&v->lock);
            return err;
        }
    }
    assert(v->read != NULL);
    v->read(out_user_data, v);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
timed_take_mvar(void* const out_user_data, void* const mvar, const long int timeout_in_msec) {
    struct timespec now;
    int err = timespec_get(&now, TIME_UTC);
    assert(err == 0);
    struct timespec tout = timespec_add(now, timeout_in_msec * 1000000);
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    err = pthread_mutex_timedlock(&v->lock, &tout);
    if (err != 0) {
        // When timer expired, ETIMEDOUT is returned.
        return err;
    }
    if (v->empty) {
        err = pthread_cond_timedwait(&v->take_cond, &v->lock, &tout);
        if (v->empty) {
            pthread_mutex_unlock(&v->lock);
            return err;
        }
    }
    assert(v->read != NULL);
    v->read(out_user_data, v);
    v->empty = true;
    pthread_cond_signal(&v->put_cond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
try_put_mvar(void* mvar, const void* const user_data)
{
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    int err = pthread_mutex_trylock(&v->lock);
    if (err != 0) {
        // return EBUSY if v->lock is already locked.
        return err;
    }
    if (!v->empty) {
        // _mvar is not empty.  Return without waiting.
        pthread_mutex_unlock(&v->lock);
        return EBUSY;
    }
    assert(v->write != NULL);
    v->write(v, user_data);
    v->empty = false;
    pthread_cond_signal(&v->take_cond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
try_read_mvar(void* const out_user_data, void* const mvar)
{
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    int err = pthread_mutex_trylock(&v->lock);
    if (err != 0) {
        // return EBUSY if v->lock is already locked.
        return err;
    }
    if (v->empty) {
        // _mvar is empty.  Return without waiting.
        pthread_mutex_unlock(&v->lock);
        return EBUSY;
    }
    assert(v->read != NULL);
    v->read(out_user_data, v);
    pthread_mutex_unlock(&v->lock);
    return 0;
}

int
try_take_mvar(void* const out_user_data, void* const mvar)
{
    assert(mvar != NULL);
    mvar_abs_t* const v = mvar;
    int err = pthread_mutex_trylock(&v->lock);
    if (err != 0) {
        // return EBUSY if v->lock is already locked.
        return err;
    }
    if (v->empty) {
        // _mvar is empty.  Return without waiting.
        pthread_mutex_unlock(&v->lock);
        return EBUSY;
    }
    assert(v->read != NULL);
    v->read(out_user_data, v);
    v->empty = true;
    pthread_cond_signal(&v->put_cond);
    pthread_mutex_unlock(&v->lock);
    return 0;
}
