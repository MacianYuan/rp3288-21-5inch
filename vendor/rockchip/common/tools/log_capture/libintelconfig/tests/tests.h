/*
 * Copyright (C) Intel 2015
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INTELCONFIG_TESTS_H
#define INTELCONFIG_TESTS_H

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int test_unescape(void);
int test_escape(void);
int json_load_integer(void);
int json_load_string(void);
int json_load_nodes(void);
int json_load_array(void);
int json_load_error(void);
int json_print_test(void);

#ifdef __cplusplus
}
#endif

#endif
