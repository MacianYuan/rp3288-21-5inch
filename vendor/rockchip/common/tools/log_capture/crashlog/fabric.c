/* Copyright (C) Intel 2013
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

/**
 * @file fabric.c
 * @brief File containing functions used to handle fabric events.
 *
 * This file contains the functions used to handle fabric events.
 */
#include "trigger.h"
#include "crashutils.h"
#include "privconfig.h"
#include "fsutils.h"

#include <stdlib.h>

struct fabric_type {
    char *keyword;
    char *tail;
    char *name;
};

struct fabric_type fabric_types_array[] = {
    {"DW0:", "f501", "MEMERR"},
    {"DW0:", "f502", "INSTERR"},
    {"DW0:", "f504", "SRAMECCERR"},
    {"DW0:", "00dd", "HWWDTLOGERR"},
    {"DW3:", "0000e101", "MEMERR"},
    {"DW3:", "0000e102", "INSTERR"},
    {"DW3:", "0000e104", "SRAMECCERR"},
    {"DW3:", "0000e106", "NORTHFUSEERR"},
    {"DW3:", "0000e10a", "KERNELHANG"},
    {"DW3:", "0000e10b", "KERNELWDT"},
    {"DW3:", "0000e10c", "SCUWDT"},
    {"DW3:", "0000e10d", "FABRICXML"},
    {"DW3:", "0000e10e", "CHAABIWDT"},
    {"DW3:", "0000e601", "PLLLOCKERR"},
    {"DW3:", "0000e603", "UNDEFL1ERR"},
    {"DW3:", "0000e604", "PUNITMBBTIMEOUT"},
    {"DW3:", "0000e605", "VOLTKERR"},
    {"DW3:", "0000e606", "VOLTSAIATKERR"},
    {"DW3:", "0000e607", "LPEINTERR"},
    {"DW3:", "0000e608", "PSHINTERR"},
    {"DW3:", "0000e609", "FUSEINTERR"},
    {"DW3:", "0000e60a", "IPC2ERR"},
    {"DW3:", "0000e60b", "KWDTIPCERR"},
};

struct fabric_type fabric_fakes_array[] = {
    {"DW2:", "02608002", "FABRIC_FAKE"},
    {"DW3:", "ff222230", "FABRIC_FAKE"},
    {"DW3:", "0000e103", "FABRIC_FAKE"},
    {"DW11:", "ff222230", "FABRIC_FAKE"},
};

enum {
    F_INFORMATIVE_MSG
};

static const struct fabric_type fabric_event[] = {
    [ F_INFORMATIVE_MSG ] = {"DW0:", "f506", "FIRMWARE"},
};

int crashlog_check_fabric(char *reason, int test) {
    const char *dateshort = get_current_time_short(1);
    char destination[PATHMAX];
    char crashtype[32] = {'\0'};
    char event_name[10] = CRASHEVENT;
    char *dir;
    unsigned int i = 0;
    char *key;

    if ( !test && !file_exists(CURRENT_PROC_FABRIC_ERROR_NAME) ) return 1;

    destination[0] = '\0';

    key = generate_event_id(event_name, crashtype);
    dir = generate_crashlog_dir(MODE_CRASH, key);

    if (!dir) {
        LOGE("%s: generate_crashlog_dir failed\n", __FUNCTION__);
        snprintf(destination, sizeof(destination), "%s", LOG_FABRICTEMP);
    } else {
        snprintf(destination, sizeof(destination), "%s/%s_%s.txt", dir,
                FABRIC_ERROR_NAME, dateshort);
    }

    do_copy_eof(CURRENT_PROC_FABRIC_ERROR_NAME, destination);

    /* Looks first for fake fabrics */
    if (( find_str_in_file(destination, fabric_fakes_array[0].keyword, fabric_fakes_array[0].tail) > 0 &&
          find_str_in_file(destination, fabric_fakes_array[1].keyword, fabric_fakes_array[1].tail) > 0 ) ||
        ( find_str_in_file(destination, fabric_fakes_array[2].keyword, fabric_fakes_array[2].tail) > 0 &&
          find_str_in_file(destination, fabric_fakes_array[3].keyword, fabric_fakes_array[3].tail) > 0 )) {
            /* Got it, it's a fake!! */
            strncpy(crashtype, fabric_fakes_array[i].name, sizeof(crashtype)-1);
            if (!strncmp(reason, "HWWDT_RESET", strlen("HWWDT_RESET")))
                strcat(reason, FAKE_EVENT_SUFFIX);
    }

    if (crashtype[0] == 0) {
        /* Not a fake, checks for the type in the know fabrics list */
        for (i = 0; i < DIM(fabric_types_array); i++) {
            if ( find_str_in_file(destination, fabric_types_array[i].keyword, fabric_types_array[i].tail) > 0 ) {
               /* Got it! */
               strncpy(crashtype, fabric_types_array[i].name, sizeof(crashtype)-1);
               if (strstr(crashtype, "HANG"))
                   strncpy(event_name, INFOEVENT, sizeof(event_name)-1);
               break;
            }
        }
        if (crashtype[0] == 0) {
            /* Not a fake but still unknown!! set a default type */
            strcpy(crashtype, FABRIC_ERROR);
        }
    }
    /* Search for INFORMATIVE_MSG reported by kernel fabric module */
    if ( find_str_in_file(destination, fabric_event[F_INFORMATIVE_MSG].keyword,
            fabric_event[F_INFORMATIVE_MSG].tail) > 0 ) {
        /* Informative_Msg from fabric -> info event */
        strncpy(event_name, INFOEVENT, sizeof(event_name)-1);
        strncpy(crashtype, fabric_event[F_INFORMATIVE_MSG].name, sizeof(crashtype)-1);
    }

    if (dir != NULL) {
        do_wdt_log_copy(dir);
        do_last_kmsg_copy(dir);
        raise_event(key, event_name, crashtype, NULL, dir);
        LOGE("%-8s%-22s%-20s%s %s\n", event_name, key, get_current_time_long(0), crashtype, dir);
        free(key);
        free(dir);
        /* Raise an aplog event to get some logs */
        if ( process_log_event(NULL, NULL, MODE_APLOGS) < 0 )
            LOGE("%s: process_log_event for APLOGS failed\n", __FUNCTION__);
        return 0;
    } else {
        raise_event(key, event_name, crashtype, NULL, NULL);
        LOGE("%-8s%-22s%-20s%s\n", event_name, key, get_current_time_long(0), crashtype);
        free(key);
        /* Remove temporary file */
        remove(LOG_FABRICTEMP);
        return 0;
    }
}
