/*
 * Copyright 2012 TennenColl
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRESHCITY_BASIC_DEBUG_LOGGING
#define FRESHCITY_BASIC_DEBUG_LOGGING

#include <log4cplus/logger.h>

extern log4cplus::Logger globalogger;

#define LOG_TRACE(x) LOG4CPLUS_TRACE(globalogger, x)
#define LOG_DEBUG(x) LOG4CPLUS_DEBUG(globalogger, x)
#define LOG_INFO(x) LOG4CPLUS_INFO(globalogger, x)
#define LOG_WARN(x) LOG4CPLUS_WARN(globalogger, x)
#define LOG_ERROR(x) LOG4CPLUS_ERROR(globalogger, x)
#define LOG_FATAL(x) LOG4CPLUS_FATAL(globalogger, x)

#endif
