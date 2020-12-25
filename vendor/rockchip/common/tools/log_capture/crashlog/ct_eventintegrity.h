/* Copyright (C) Intel 2014
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
 * @file eventintegrity.h
 * @brief File containing functions used to check event files integrity.
 *
 * This file contains the functions used to check event files integrity.
 */

#ifndef __EVENTINTEGRITY_H__
#define __EVENTINTEGRITY_H__

#ifndef CRASHLOGD_MODULE_KCT
#include <kct_stub.h>
#else
#include <linux/kct.h>
#endif

int check_event_integrity(struct ct_event* ev, char* event_dir);

#endif /* __EVENTINTEGRITY_H__ */
