/* * Copyright (C) Intel 2010
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
 * @file mmgr_source.h
 * @brief File containing functions to handle modem manager source and events
 * coming from this source.
 *
 * This file contains the functions to handle the modem manager source (init,
 * closure, events reading from socket) and the functions to process the
 * different kinds of events read from this source.
 */

#ifndef MMGR_SOURCE_H_INCLUDED
#define MMGR_SOURCE_H_INCLUDED

#ifdef CRASHLOGD_MODULE_MODEM
#include <stdlib.h>

int mmgr_get_fd(unsigned int mdm_inst);
void init_mmgr_cli_source(unsigned int mdm_inst);
void close_mmgr_cli_source(unsigned int mdm_inst);
int mmgr_handle(unsigned int mdm_inst);
#else
static inline void init_mmgr_cli_source(unsigned int __attribute((unused))
                                        mdm_inst) {}
static inline void close_mmgr_cli_source(unsigned int __attribute((unused))
                                         mdm_inst) {}
static inline int mmgr_get_fd(unsigned int __attribute((unused)) mdm_inst) {
    return 0;
}
static inline int mmgr_handle(unsigned int __attribute((unused)) mdm_inst) {
    return 0;
}
#endif

#endif
