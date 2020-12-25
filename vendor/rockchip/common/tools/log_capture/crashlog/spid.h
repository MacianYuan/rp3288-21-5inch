/*
 * Copyright (C) Intel 2014
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

/**
 * Read SPID data from sysfs, build it and write it into given file
 */
#ifdef CRASHLOGD_MODULE_SPID
void read_sys_spid(const char *filename);
#else
static inline void read_sys_spid(const char *filename __attribute__((__unused__))) {}
#endif
