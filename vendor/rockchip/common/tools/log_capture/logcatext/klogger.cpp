/* * Copyright (C) Intel 2014-2015
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "klogger.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <log/logger.h>

#define FPATH "/dev/kmsg"
#define K_LOG_TAG "KERNEL"
#define KLOGGER_ID LOG_ID_MAX

#define USEC_IN_SEC 1000000L
#define NSEC_IN_USEC 1000L
#define NSEC_IN_SEC (NSEC_IN_USEC * USEC_IN_SEC)
#define SEC_FROM_USEC(a) ((a)/(USEC_IN_SEC))
#define EXTRA_NSEC(a)    (((a)%(USEC_IN_SEC))*NSEC_IN_USEC)
#define EXTRA_USEC(a)    ((a)%(USEC_IN_SEC))

#define POLL_TIMEOUT 100000

static int fd = -1;
static struct timespec ts;
static unsigned char priority;

static void init_msg_header(struct logger_entry_v3 *hdr) {
    hdr->pid = 0;
    hdr->tid = 0;
    hdr->lid = KLOGGER_ID;
    hdr->hdr_size = sizeof(struct logger_entry_v3);
};

static void setup_msg_header(struct logger_entry_v3 *hdr, unsigned int len) {
    hdr->len = len;
    hdr->sec = ts.tv_sec;
    hdr->nsec = ts.tv_nsec;
    hdr->msg[0] = priority;
};

static void ts_norm(struct timespec *pts, time_t sec, long long nsec) {
    /* Carry over available seconds */
    if (nsec >= NSEC_IN_SEC || nsec <= -NSEC_IN_SEC) {
        sec += nsec / NSEC_IN_SEC;
        nsec %= NSEC_IN_SEC;
    }

    /*Try to keep the same sign*/
    if (sec < 0 && nsec > 0) {
        nsec -= NSEC_IN_SEC;
        sec++;
    }

    if (sec > 0 && nsec < 0) {
        nsec += NSEC_IN_SEC;
        sec--;
    }

    pts->tv_sec = sec;
    pts->tv_nsec = nsec;
}

static struct timespec ts_add(struct timespec ts1, struct timespec ts2) {
    struct timespec ret;
    ts_norm(&ret, ts1.tv_sec + ts2.tv_sec, ts1.tv_nsec + ts2.tv_nsec);
    return ret;
}

static struct timespec ts_sub(struct timespec ts1, struct timespec ts2) {
    struct timespec ret;
    ts_norm(&ret, ts1.tv_sec - ts2.tv_sec, ts1.tv_nsec - ts2.tv_nsec);
    return ret;
}

static struct timespec ts_add_us(struct timespec ts1, unsigned long long usec) {
    struct timespec ret;
    ts_norm(&ret, ts1.tv_sec, ts1.tv_nsec + (usec * NSEC_IN_USEC));
    return ret;
}

static void update_header_data(unsigned int prio) {
    struct timespec ts_real;
    clock_gettime(CLOCK_REALTIME, &ts_real);
    /*
    * Time calculations proved to be very tricky. Apparently the closest userspace
    * time source to the one used by the kernel logger is boottime. But even with
    * this one, some drifts could be observed in long time running use cases.
    */
    ts.tv_sec = ts_real.tv_sec;
    ts.tv_nsec = ts_real.tv_nsec;


    switch (prio & 7) {
    case 0:                    /*KERN_EMERG */
    case 1:                    /*KERN_ALERT */
    case 2:                    /*KERN_CRIT */
        priority = ANDROID_LOG_FATAL;
        break;
    case 3:                    /*KERN_ERR */
        priority = ANDROID_LOG_ERROR;
        break;
    case 4:                    /*KERN_WARNING */
        priority = ANDROID_LOG_WARN;
        break;
    case 5:                    /*KERN_NOTICE */
    case 6:                    /*KERN_INFO */
        priority = ANDROID_LOG_INFO;
        break;
    case 7:                    /*KERN_DEBUG */
        priority = ANDROID_LOG_DEBUG;
        break;
    }
}

static void update_header_data(unsigned long long usec, unsigned int prio) {
    struct timespec ts_real, ts_ref;
    clock_gettime(CLOCK_REALTIME, &ts_real);
    /*
    * Time calculations proved to be very tricky. Apparently the closest userspace
    * time source to the one used by the kernel logger is boottime. But even with
    * this one, some drifts could be observed in long time running use cases.
    */
    clock_gettime(CLOCK_BOOTTIME, &ts_ref);

    ts = ts_sub(ts_real, ts_ref);
    ts = ts_add_us(ts, usec);

    switch (prio & 7) {
    case 0:                    /*KERN_EMERG */
    case 1:                    /*KERN_ALERT */
    case 2:                    /*KERN_CRIT */
        priority = ANDROID_LOG_FATAL;
        break;
    case 3:                    /*KERN_ERR */
        priority = ANDROID_LOG_ERROR;
        break;
    case 4:                    /*KERN_WARNING */
        priority = ANDROID_LOG_WARN;
        break;
    case 5:                    /*KERN_NOTICE */
    case 6:                    /*KERN_INFO */
        priority = ANDROID_LOG_INFO;
        break;
    case 7:                    /*KERN_DEBUG */
        priority = ANDROID_LOG_DEBUG;
        break;
    }
}

bool klogger_init() {
    fd = open(FPATH, O_RDONLY | O_NONBLOCK);
    if (fd < 0)
        return false;

    return true;
}

void klogger_destroy() {
    if (fd >= 0)
        close(fd);
    return;
}

int klogger_read(struct log_msg *log_msg) {
    static char line_buffer[LOGGER_ENTRY_MAX_LEN];
    size_t ret, payload_length;
    char *msg_start;
    unsigned long long usec;
    unsigned int prio;

    init_msg_header(&log_msg->entry_v3);

    ret = read(fd, line_buffer, LOGGER_ENTRY_MAX_LEN);
    if (ret <= 0) {
        return ret;
    }
    /*just make shure we are null terminated */
    if (ret < LOGGER_ENTRY_MAX_LEN)
        line_buffer[ret] = 0;
    else
        line_buffer[LOGGER_ENTRY_MAX_LEN - 1] = 0;

    msg_start = strchr(line_buffer, ';');
    /*the first char will contain the priority */
    payload_length = sprintf(log_msg->msg(), "P%s", K_LOG_TAG);

    if (msg_start) {
        msg_start++;
        if (sscanf(line_buffer, "%u,%*u,%llu,", &prio, &usec) == 2) {
            /*fill the header */
            //update_header_data(usec, prio);
            update_header_data(prio);
            payload_length += sprintf(log_msg->msg() + payload_length + 1,
                                      "[%5llu.%06llu] %s", SEC_FROM_USEC(usec),
                                      EXTRA_USEC(usec), msg_start);
        } else {
            payload_length += sprintf(log_msg->msg() + payload_length + 1,
                                      "<pe> %s", msg_start);
        }
    } else
        payload_length += sprintf(log_msg->msg() + payload_length + 1,
                                  "<pe> %s", line_buffer);

    setup_msg_header(&log_msg->entry_v3, payload_length + 2);
    return ret;
}

int klogger_wait() {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN | POLLPRI;
    return poll(&pfd, 1, POLL_TIMEOUT);
}
